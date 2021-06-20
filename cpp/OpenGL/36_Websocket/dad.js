var activeRealm = 0;
var brushSize = 10;
var currentTarget = 2;
var socket;
var serverWidth;
var serverHeight;
var h,s,v;
var hiddenCanvases = [];

var color = {r:255,g:0,b:0,a:255};
const eraser = {r:0,g:0,b:0,a:0};

function createCanvas(width, height) {
  let c = document.createElement('canvas');
  c.setAttribute('width', width);
  c.setAttribute('height', height);
  return c;
}

function HSVtoRGB(h, s, v) {
  let r, g, b, i, f, p, q, t;
  i = Math.floor(h * 6);
  f = h * 6 - i;
  p = v * (1 - s);
  q = v * (1 - f * s);
  t = v * (1 - (1 - f) * s);
  switch (i % 6) {
    case 0: r = v, g = t, b = p; break;
    case 1: r = q, g = v, b = p; break;
    case 2: r = p, g = v, b = t; break;
    case 3: r = p, g = q, b = v; break;
    case 4: r = t, g = p, b = v; break;
    case 5: r = v, g = p, b = q; break;
  }
  return {
    r: Math.round(r * 255),
    g: Math.round(g * 255),
    b: Math.round(b * 255),
    a: 255
  };
}

function rgb(r, g, b){
  return ["rgb(",r,",",g,",",b,")"].join("");
}

function deserializeMessage(buffer) {
  if (buffer instanceof Uint8Array) {
    let parser = new ArrayParser(buffer);
    let type = parser.nextUint8();
    switch (type) {
      case 0 :
        return InitMessage.deserialize(parser);
      case 1 :
        return PaintMessage.deserialize(parser);
      case 2 :
        return ClearMessage.deserialize(parser);
      default:
        throw new Error("invalid message type");
    };
  } else {
    throw new Error("invalid message buffer");
  }
}
      
class ArrayBuilder {
  constructor() {
    this.buffer = [];
  }
  
  addUint8(num) {
    this.buffer.push(num & 0xff);
  }
  
  addUint16(num) {
    this.buffer.push((num >> 8) & 0xff);
    this.buffer.push(num & 0xff);
  }

  addUint32(num) {
    this.buffer.push((num >> 24) & 0xff);
    this.buffer.push((num >> 16) & 0xff);
    this.buffer.push((num >> 8) & 0xff);
    this.buffer.push(num & 0xff);
  }
  
  getArray() {
    return new Uint8Array(this.buffer);
  }
}

class ArrayParser {
  constructor(buffer) {
    this.buffer = buffer;
    this.pos = 0;
  }
  
  nextUint8() {
    if (this.pos >= this.buffer.length) throw new Error("invalid message length");
    return this.buffer[this.pos++];
  }
  
  nextUint16() {
    let a = this.nextUint8();
    let b = this.nextUint8();
    return (a << 8) + b;
  }
  
  nextUint32() {
    let a = this.nextUint8();
    let b = this.nextUint8();
    let c = this.nextUint8();
    let d = this.nextUint8();
    return (a << 24) + (b << 16) + (c << 8) + d;
  }
  
  nextBytes(count) {
    if (this.pos+count > this.buffer.length) throw new Error("invalid message length");
    let start = this.pos;
    this.pos += count;
    return this.buffer.slice(start, start+count);
    
  }
}

class PaintMessage {
  constructor(posX, posY, r, g, b, a, brushSize, target) {
    this.posX = posX;
    this.posY = posY;
    this.r = r;
    this.g = g;
    this.b = b;
    this.a = a;
    this.brushSize = brushSize;
    this.target = target;
  }
  
  serialize() {
    let builder = new ArrayBuilder();
    builder.addUint8(1);
    builder.addUint32(activeRealm);
    builder.addUint16(this.posX);
    builder.addUint16(this.posY);
    builder.addUint8(this.r);
    builder.addUint8(this.g);
    builder.addUint8(this.b);
    builder.addUint8(this.a);
    builder.addUint16(this.brushSize);
    builder.addUint8(this.target);
    return builder.getArray();
  }
          
  static deserialize(parser) {
    let realm = parser.nextUint32();
    let posX = parser.nextUint16();
    let posY = parser.nextUint16();
    let r = parser.nextUint8();
    let g = parser.nextUint8();
    let b = parser.nextUint8();
    let a = parser.nextUint8();
    let brushSize = parser.nextUint16();
    let target = parser.nextUint8();
    return new PaintMessage(posX, posY, r, g, b, a, brushSize, target);
  }
};

class ClearMessage {
  constructor(r, g, b, a, target) {
    this.r = r;
    this.g = g;
    this.b = b;
    this.a = a;
    this.target = target;
  }
  
  serialize() {
    let builder = new ArrayBuilder();
    builder.addUint8(2);
    builder.addUint32(activeRealm);
    builder.addUint8(this.r);
    builder.addUint8(this.g);
    builder.addUint8(this.b);
    builder.addUint8(this.a);
    builder.addUint8(this.target);
    return builder.getArray();
  }
  
  static deserialize(parser) {
    let realm = parser.nextUint32();
    let r = parser.nextUint8();
    let g = parser.nextUint8();
    let b = parser.nextUint8();
    let a = parser.nextUint8();
    let target = parser.nextUint8();
    return new ClearMessage(r, g, b, a, target);
  }
};

class InitMessage {
  constructor(imageData, width, height, layerCount) {
    this.width = width;
    this.height = height;
    this.imageData = imageData;
    this.layerCount = layerCount;
  }
          
  static deserialize(parser) {
    let width = parser.nextUint16();
    let height = parser.nextUint16();
    let layerCount = parser.nextUint8();
    let data = parser.nextBytes(width*height*layerCount*4);
    return new InitMessage(data, width, height, layerCount);
  }
};
