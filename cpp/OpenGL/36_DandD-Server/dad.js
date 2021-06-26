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
  constructor(realm, posX, posY, r, g, b, a, brushSize, target) {
    this.realm = realm;
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
    return new PaintMessage(realm, posX, posY, r, g, b, a, brushSize, target);
  }
};

class ClearMessage {
  constructor(realm, r, g, b, a, target) {
    this.realm = realm;
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
    return new ClearMessage(realm, r, g, b, a, target);
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

var mouseX, mouseY, leftMouseDown=false, rightMouseDown=false;
var shiftPressed = false;
var canvas, ctx;

function processBuffer(buffer) {
  try {
    let message = deserializeMessage(buffer);
    switch (message.constructor) {
      case PaintMessage: {
        if (message.realm != activeRealm) return;
        let layerCtx = hiddenCanvases[message.target].getContext('2d');
        let sprite = layerCtx.createImageData(message.brushSize, message.brushSize);
        for (let y = 0; y < sprite.height; ++y) {
          for (let x = 0; x < sprite.width; ++x) {
            let i = (x+y*sprite.width)*4;
            sprite.data[i + 0] = message.r;
            sprite.data[i + 1] = message.g;
            sprite.data[i + 2] = message.b;
            sprite.data[i + 3] = message.a;
          }
        }
        layerCtx.putImageData(sprite, message.posX, message.posY);
        compose();
        break;
      }
      case InitMessage: {
        cursors.clear();
        hiddenCanvases = [];
        serverWidth = message.width;
        serverHeight = message.height;
        
        for (let l = 0;l<message.layerCount;++l) {
          hiddenCanvases.push(createCanvas(message.width,message.height));
          let layerCtx = hiddenCanvases[hiddenCanvases.length-1].getContext('2d');
          let initialImageData = layerCtx.createImageData(message.width, message.height);
          const offset = l * message.width*message.height*4;
          
          for (let i = 0;i<message.width*message.height*4;i++) {
            initialImageData.data[i] = message.imageData[i+offset];
          }
          
          layerCtx.putImageData(initialImageData, 0, 0);
        }
        
        positionCanvas = createCanvas(message.width,message.height);
        activeRealmName = message.name;
        activeRealm = message.id;
        
        for (let i = 0; i < message.cursors.length; ++i) {
          cursors.set(message.cursors[i].id, [message.cursors[i].name, message.cursors[i].posX, message.cursors[i].posY, message.id]);
        }
        updatePositionMarkers();
        compose();
        document.getElementById("realmText").value = activeRealm;
        document.getElementById("title").innerHTML = "Dungeon Master UI (" + activeRealmName + ")";
        break;
      }
      case ClearMessage: {
        if (message.realm != activeRealm) return;
        
        let layerCtx = hiddenCanvases[message.target].getContext('2d');
        let sprite = layerCtx.createImageData(hiddenCanvases[message.target].width, hiddenCanvases[message.target].height);
        for (let y = 0; y < hiddenCanvases[message.target].height; ++y) {
          for (let x = 0; x < hiddenCanvases[message.target].width; ++x) {
            let i = (x+y*sprite.width)*4;
            sprite.data[i + 0] = message.r;
            sprite.data[i + 1] = message.g;
            sprite.data[i + 2] = message.b;
            sprite.data[i + 3] = message.a;
          }
        }
        layerCtx.putImageData(sprite, 0,0);
        compose();
        break;
      }
      case PositionMessage: {
        if (message.realm == activeRealm)  {
          cursors.set(message.id, [message.name, message.posX, message.posY, message.realm]);
          updatePositionMarkers();
          compose();
        } else {
          if (cursors.delete(message.id)) {
            updatePositionMarkers();
            compose();                
          }
        }
        break;
      }
    }
  } catch (e) {
    console.log(e)
  }
}

function overrideLayer(){
  if(socket.readyState == 1)
    socket.send(new ClearMessage(activeRealm,color.r, color.g, color.b, color.a, currentTarget).serialize());
}

function eraseLayer(){
  if(socket.readyState == 1)
    socket.send(new ClearMessage(activeRealm,0,0,0,0,currentTarget).serialize());
}

function applyColor() {
  let hueLabel = document.getElementById("hueLabel");
  color = HSVtoRGB(h,s,v);
  hueLabel.style.backgroundColor = rgb(color.r,color.g,color.b);
}


function setPosition() {
  if(socket.readyState == 1) {
    socket.send(new PositionMessage(activeRealm, 0, serverWidth * mouseX/canvas.width,
      serverHeight * mouseY/canvas.height,
      name).serialize());
  }
}

function dropPaint(rgba) {
  if(socket.readyState == 1)
    socket.send(new PaintMessage(activeRealm, serverWidth * mouseX/canvas.width,
      serverHeight * mouseY/canvas.height,
      rgba.r, rgba.g, rgba.b, rgba.a, 
      Math.round(brushSize), currentTarget).serialize());
}

function handleContextMenu() {
  event.preventDefault();
  return false;
}

function handleMouseDown(event) {
  if (event.button === 0) {
    leftMouseDown = true;
    dropPaint(color);
  }
  if (event.button === 2) {
    rightMouseDown = true;
    dropPaint(eraser);
  }
  
  event.preventDefault();
}

function handleMouseUp(event) {
  if (event.button === 0)
    leftMouseDown = false;
  if (event.button === 2)
    rightMouseDown = false;
  event.preventDefault();
}

function handleMouseMove(event) {
  var rect = event.target.getBoundingClientRect();
  mouseX = event.clientX - rect.left;
  mouseY = event.clientY - rect.top;
  event.preventDefault();
  if (leftMouseDown) dropPaint(color);
  if (rightMouseDown) dropPaint(eraser);
  if (shiftPressed) setPosition();
}


function updatePositionMarkers() {
  // TODO: consider resizing "positionCanvas" along with "canvas" and adjusting the coordinates
  
  let layerCtx = positionCanvas.getContext('2d');
  layerCtx.clearRect(0, 0, positionCanvas.width, positionCanvas.height);
  layerCtx.font = "20px Arial";
  layerCtx.fillStyle = 'black';
  for (const [key, value] of cursors.entries()) {
    layerCtx.beginPath();
    layerCtx.arc(value[1], value[2], 5, 0, 2 * Math.PI, false);
    layerCtx.fill();
    layerCtx.fillText(value[0], value[1]+8, value[2]-8);
  }
}
