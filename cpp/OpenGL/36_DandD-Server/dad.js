var activeRealm;
var activeRealmName = "Lobby";
var brushSize;
var currentTarget;
var socket;
var serverWidth;
var serverHeight;
var h,s,v;
var hiddenCanvases = [];
var positionCanvas;
var name;
var cursors = new Map();


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
      case 3 :
        return PositionMessage.deserialize(parser);
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

  addString(str) {
    this.addUint32(str.length);
    for (let i = 0;i<str.length;++i) {
      this.addUint8(str.charCodeAt(i));
    }
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

  nextString() {
    let str = "";
    let l = this.nextUint32();
    for (let i = 0;i<l;++i) {
      let n = this.nextUint8();
      str += String.fromCharCode(n);
    }
    return str;
  }
};

class ChangeRealmMessage {
  constructor(realm, id) {
    this.realm = realm;
    this.id = id;
  }
  
  serialize() {
    let builder = new ArrayBuilder();
    builder.addUint8(4);
    builder.addUint32(this.realm);
    builder.addUint32(this.id);
    return builder.getArray();
  }
  
  static deserialize(parser) {
    let realm = parser.nextUint32();
    let id    = parser.nextUint32();
    return new ChangeRealmMessage(realm, id);
  }
  
};

class PositionMessage {
  constructor(realm, id, posX, posY, name) {
    this.realm = realm;
    this.id = id;
    this.posX = posX;
    this.posY = posY;
    this.name = name;
  }
  
  serialize() {
    let builder = new ArrayBuilder();
    builder.addUint8(3);
    builder.addUint32(this.realm);
    builder.addUint32(this.id);
    builder.addUint16(this.posX);
    builder.addUint16(this.posY);
    builder.addString(this.name);
    return builder.getArray();
  }
  
  static deserialize(parser) {
    let realm = parser.nextUint32();
    let id    = parser.nextUint32();
    let posX  = parser.nextUint16();
    let posY  = parser.nextUint16();
    let name  = parser.nextString();
    return new PositionMessage(realm, id, posX, posY, name);
  }
};

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
  constructor(imageData, width, height, layerCount, name, id, cursors) {
    this.width = width;
    this.height = height;
    this.imageData = imageData;
    this.layerCount = layerCount;
    this.name = name
    this.id = id;
    this.cursors = cursors;
  }
          
  static deserialize(parser) {
    let width = parser.nextUint16();
    let height = parser.nextUint16();
    let layerCount = parser.nextUint8();
    let name = parser.nextString();
    let id = parser.nextUint32();
    let cursors = [];
    let cursorCount = parser.nextUint32();
    for (let cursor = 0;cursor<cursorCount;++cursor) {
      let cursorID = parser.nextUint32()
      let cursorX = parser.nextUint16();
      let cursorY = parser.nextUint16();
      let cursorName = parser.nextString();
      cursors.push({id:cursorID,name:cursorName,posX:cursorX, posY:cursorY});
    }
    let data = parser.nextBytes(width*height*layerCount*4);
    return new InitMessage(data, width, height, layerCount, name, id, cursors);
  }
};

function createSocket() {
  socket = new WebSocket('ws://192.168.52.205:2000');
  //socket = new WebSocket('ws://134.91.11.186:2000');
  //socket = new WebSocket('ws://134.91.11.162:2000');
  
  socket.onopen = function () {
    document.getElementById("status").innerHTML = "Online<br>";
  };
  
  socket.onmessage = function (msg) {
    if (typeof msg.data === "string")
      document.getElementById("status").innerHTML = "Received String Message: " + msg.data +"<br>";
    else {
      let reader = new FileReader();
      reader.readAsArrayBuffer(msg.data);
      reader.addEventListener("loadend", function(e) {
        processBuffer(new Uint8Array(e.target.result))
      });
    }
  };
  
  socket.onclose = function() {
    document.getElementById("status").innerHTML = "Offline<br>";
    createSocket();
  };
}

function genRandomName() {
  var firstName= [
    "Rhazun", "Hehlem", "Brergem",
    "Grubrog", "Bordam", "Sir",
    "Stradrun", "Mon", "Rihkor-Deh",
    "Ga-Zuk", "Jerindac", "Kadjor",
    "Chip", "Juiw", "Gruernavan",
    "Granraz", "Nekhom", "Reihre",
    "Nehnud", "Nezen", "Stoe",
    "Mabrin", "Pam", "Ugrum",
    "Grur", "Gi-Ka-Vas", "Lar-Kid",
    "Drirdatvak", "Tamzaur", "Lu",
    "Mu", "Diveantis", "Crinchain"
  ];
  
  var lastName   = [
    "Bassil", "Bamme", "Wiseshot",
    "Solidcrusher", "Nen", "Mivatsk",
    "Cragspirit", "Lonelash", "Nivriltrum",
    "Mankrid", "Urguguge", "Vrabure",
    "Ciy", "Kua", "Zodrastan",
    "Rerguro", "Nekhom", "Reihre",
    "Commonarm", "Starlance", "Grask",
    "Kedrav", "Hallowdream", "Vuhpuukt",
    "Grimadotve", "Hobuma", "Mung",
    "Rerguro", "Nekhom", "Reihre",
    "Miao", "Banucas", "Desculir"
  ];

  return firstName[Math.floor(Math.random() * firstName.length)] + " " + 
         lastName[Math.floor(Math.random() * lastName.length)];
}


function saveState() {
  document.cookie = "name=" + name;
  document.cookie = "activeRealm=" + activeRealm;
  document.cookie = "h=" + h;
  document.cookie = "s=" + s;
  document.cookie = "v=" + v;
  document.cookie = "currentTarget=" + currentTarget;
  document.cookie = "brushSize=" + brushSize;
}

function loadState() {
  name = getCookie("name");
  activeRealm = getCookie("activeRealm");
  h = getCookie("h");
  s = getCookie("s");
  v = getCookie("v");
  currentTarget = getCookie("currentTarget");
  brushSize = getCookie("brushSize");
  
  if (activeRealm === "") {
    activeRealm = 0;
    brushSize = 10;
    currentTarget = 2;
    h = Math.round(Math.random());
    s = 1;
    v = 1;
    name = genRandomName();
  }
}

function getCookie(cname) {
  let name = cname + "=";
  let decodedCookie = decodeURIComponent(document.cookie);
  let ca = decodedCookie.split(';');
  for(let i = 0; i <ca.length; i++) {
    let c = ca[i].trim();
    if (c.indexOf(name) == 0) {
      return c.substring(name.length, c.length);
    }
  }
  return "";
}