#include "EchoServer.h"

EchoServer::EchoServer(uint16_t port, uint16_t canvasWidth, uint16_t canvasHeight,
                       const std::vector<std::string>& layerImages) :
Server(port),
canvasWidth(canvasWidth),
canvasHeight(canvasHeight),
layerImages(layerImages)
{

  if (layerImages.size() > 255) {
    throw EchoServerException("Too many layers");
  }
  
  imageMessage.resize(initMessageHeaderSize() + initMessageLayerSize() * layerImages.size());
  
  BinaryEncoder enc;
  enc.add(uint8_t(0));
  enc.add(canvasWidth);
  enc.add(canvasHeight);
  enc.add(uint8_t(layerImages.size()));
  imageMessage = enc.getEncodedMessage();
  imageMessage.resize(initMessageHeaderSize() + initMessageLayerSize() * layerImages.size(), 0);
   
  for (size_t layerIndex = 0;layerIndex<layerImages.size();layerIndex++) {
    loadPaintLayer(layerIndex);
  }  
}
  
EchoServer::~EchoServer() {
  savePaintLayers();
}
 
void EchoServer::savePaintLayers() {
  for (size_t layerIndex = 0;layerIndex<layerImages.size();layerIndex++) {
    savePaintLayer(layerIndex);
  }
}

size_t EchoServer::initMessageHeaderSize() const {
  return 6;
}

size_t EchoServer::initMessageLayerSize() const {
  return size_t(canvasWidth) * size_t(canvasHeight) * 4;
}

void EchoServer::savePaintLayer(size_t layerIndex) {
  const std::string filename = layerImages[layerIndex];
  
  std::cout << "Saving layer " << filename << std::flush;
  imageLock.lock();
  
  const long layerOffset = long(initMessageHeaderSize() + initMessageLayerSize() * layerIndex);
  std::vector<uint8_t> data{imageMessage.begin()+layerOffset,
                            imageMessage.begin()+layerOffset+long(initMessageLayerSize())};
  imageLock.unlock();

  try {
    const Image i = Image(canvasWidth, canvasHeight, 4, data);
    BMP::save(filename, i.flipHorizontal());
    std::cout << " done." << std::endl;
  } catch (...) {
    std::cerr << " FAILED." << std::endl;
  }
}

void EchoServer::loadPaintLayer(size_t layerIndex) {
  const std::string filename = layerImages[layerIndex];
  
  std::cout << "Loading layer " << filename <<std::flush;

  const size_t layerOffset = initMessageHeaderSize() + initMessageLayerSize() * layerIndex;

  try {
    Image p = BMP::load(filename);
    p = p.flipHorizontal();
    
    if (canvasWidth == p.width && canvasHeight == p.height &&
        (3 == p.componentCount || 4 == p.componentCount)) {

      if (4 == p.componentCount) {
        for (size_t i = 0;i<initMessageLayerSize();++i) {
          imageMessage[layerOffset+i] = p.data[i];
        }
      } else {
        for (size_t i = 0;i<initMessageLayerSize()/4;i++) {
          imageMessage[layerOffset+i*4+0] = p.data[i*3+0];
          imageMessage[layerOffset+i*4+1] = p.data[i*3+1];
          imageMessage[layerOffset+i*4+2] = p.data[i*3+2];
          imageMessage[layerOffset+i*4+3] = 255;
        }
      }
      
      std::cout << " done." << std::endl;
    } else {
      throw BMP::BMPException("Invalid Image dimensions");
    }
  } catch (...) {
    std::fill(imageMessage.begin()+long(layerOffset), imageMessage.begin()+long(layerOffset+initMessageLayerSize()), 0);
    std::cerr << " failed. Created new empty layer instead." << std::endl;
  }
}

void EchoServer::handleClientMessage(uint32_t id, const std::string& message) {
  std::cerr << "Error: Client (id:" << id << ") send a string message" << std::endl;
}

std::vector<uint8_t> EchoServer::fixPaintMessage(const std::vector<uint8_t> &message) {
  try {
    BinaryDecoder dec(message);
    uint8_t id = dec.nextUint8();
    uint16_t posX = std::clamp<uint16_t>(dec.nextUint16(), 0, canvasWidth-1);
    uint16_t posY = std::clamp<uint16_t>(dec.nextUint16(), 0, canvasWidth-1);
    uint8_t r = dec.nextUint8();
    uint8_t g = dec.nextUint8();
    uint8_t b = dec.nextUint8();
    uint8_t a = dec.nextUint8();
    uint16_t size = std::clamp<uint16_t>(dec.nextUint16(),1,200);
    uint8_t target = dec.nextUint8();

    if (id != 1 || target >= layerImages.size()) throw EchoServerException("Not a valid paint message");

    BinaryEncoder enc;
    enc.add(id);
    enc.add(posX);
    enc.add(posY);
    enc.add(r);
    enc.add(g);
    enc.add(b);
    enc.add(a);
    enc.add(size);
    enc.add(target);
    return enc.getEncodedMessage();
  } catch (...) {
    throw EchoServerException("Not a valid paint message");
  }
}

void EchoServer::handleClientMessage(uint32_t id, const std::vector<uint8_t>& message) {
  if (message.empty()) return;

  switch (message[0]) {
    case 1 :
      imageLock.lock();
      paint(message);
      imageLock.unlock();
      try {
        sendMessage(fixPaintMessage(message));
      } catch (const EchoServerException& e) {
        std::cout << "Client " << id << " is sending invalid data '"
                  << e.what() << "'"<< std::endl;
      }
      break;
    case 2 :
      imageLock.lock();
      clear(message);
      imageLock.unlock();
      try {
        sendMessage(message);
      } catch (const EchoServerException& e) {
        std::cout << "Client " << id << " is sending invalid data '"
                  << e.what() << "'"<< std::endl;
      }
      break;
  }
}

void EchoServer::handleClientConnection(uint32_t id, const std::string& address, uint16_t port) {
  std::cout << "New client (id:" << id << ") connected from " << address << std::endl;
  printStats();
  sendMessage(imageMessage,id);
}

void EchoServer::handleClientDisconnection(uint32_t id) {
  std::cout << "Client (id:" << id << ") disconnected" << std::endl;
  printStats();
}

void EchoServer::handleProtocolMessage(uint32_t id, uint32_t messageID, const std::vector<uint8_t>& message) {
  std::cout << "Protocol Message 0x" << std::hex << messageID << std::dec << " form id " << id << std::endl;
}

void EchoServer::handleError(const std::string& message) {
  std::cerr << "Error: " << message << std::endl;
}

void EchoServer::printStats() {
  std::cout << "A total of " << getValidIDs().size() << " clients are currently connected." << std::endl;
}
  
uint16_t EchoServer::to16Bit(const std::vector<uint8_t>& message, size_t index) {
  return uint16_t(uint16_t(message[index]) << 8) | message[index+1];
}

void EchoServer::clear(const std::vector<uint8_t>& message) {
  
  try {
    BinaryDecoder dec(message);
    uint8_t id = dec.nextUint8();
    uint8_t r = dec.nextUint8();
    uint8_t g = dec.nextUint8();
    uint8_t b = dec.nextUint8();
    uint8_t a = dec.nextUint8();
    uint8_t target = dec.nextUint8();
    
    if (id != 2 || target >= layerImages.size()) return;
  
    for (uint32_t i = 0;i<canvasHeight*canvasWidth;++i) {
      const size_t serialPos = size_t(i)*4+(initMessageLayerSize()*target) + initMessageHeaderSize();
      imageMessage[serialPos+0] = r;
      imageMessage[serialPos+1] = g;
      imageMessage[serialPos+2] = b;
      imageMessage[serialPos+3] = a;
    }
  } catch (...) {}
}

void EchoServer::paint(const std::vector<uint8_t>& message) {
  
  try {
    BinaryDecoder dec(message);
    uint8_t id = dec.nextUint8();
    uint16_t brushCenterPosX = std::clamp<uint16_t>(dec.nextUint16(), 0, canvasWidth-1);
    uint16_t brushCenterPosY = std::clamp<uint16_t>(dec.nextUint16(), 0, canvasWidth-1);
    uint8_t r = dec.nextUint8();
    uint8_t g = dec.nextUint8();
    uint8_t b = dec.nextUint8();
    uint8_t a = dec.nextUint8();
    uint16_t brushSize = std::clamp<uint16_t>(dec.nextUint16(),1,200);
    uint8_t target = dec.nextUint8();
  
    if (id != 1 || target >= layerImages.size()) return;
      for (uint32_t y = 0;y<brushSize;++y) {
        for (uint32_t x = 0;x<brushSize;++x) {
          const uint32_t posX = brushCenterPosX+x;
          const uint32_t posY = brushCenterPosY+y;
          if (posX < canvasWidth && posY < canvasHeight) {
            const size_t serialPos = size_t(posX + posY * canvasWidth)*4+(initMessageLayerSize()*target) + initMessageHeaderSize();
            imageMessage[serialPos+0] = r;
            imageMessage[serialPos+1] = g;
            imageMessage[serialPos+2] = b;
            imageMessage[serialPos+3] = a;
          }
        }
      }
  } catch (...) {}
}
