#include "EchoServer.h"

EchoServer::EchoServer(uint16_t port, uint16_t canvasWidth, uint16_t canvasHeight, uint8_t layerCount) :
Server(port),
canvasWidth(canvasWidth),
canvasHeight(canvasHeight),
layerCount(layerCount)
{
  imageMessage.resize(size_t(canvasWidth)*size_t(canvasHeight)*4*size_t(layerCount)+5);
  imageMessage[0] = 0;
  imageMessage[1] = (canvasWidth >> 8) & 0xff;
  imageMessage[2] = canvasWidth & 0xff;
  imageMessage[3] = (canvasHeight >> 8) & 0xff;
  imageMessage[4] = canvasHeight & 0xff;
   
  loadPaintLayer("paint.bmp");
}
  
EchoServer::~EchoServer() {
  savePaintLayer("paint.bmp");
}
  
void EchoServer::savePaintLayer(const std::string& filename) {
  std::cout << "Saving paint" << std::endl;
  imageLock.lock();
  std::vector<uint8_t> data{imageMessage.begin()+long(5), imageMessage.begin()+long(5+size_t(canvasWidth)*size_t(canvasHeight)*4)};
  imageLock.unlock();

  try {
    BMP::save(filename, Image(canvasWidth, canvasHeight, 4, data));
    std::cout << "Paint saved" << std::endl;
  } catch (...) {
    std::cerr << "Paint could not be saved" << std::endl;
  }
}

void EchoServer::loadPaintLayer(const std::string& filename) {
  std::cout << "Loading paint" << std::endl;
  try {
    Image p = BMP::load(filename);
    if (canvasWidth == p.width && canvasHeight == p.height && 4 == p.componentCount) {

      for (size_t i = 0;i<size_t(canvasWidth)*size_t(canvasHeight)*4;++i) {
        imageMessage[5+i] = p.data[i];
      }
      
      std::cout << "Paint Loaded" << std::endl;
    } else {
      throw BMP::BMPException("Invalid Image dimensions");
    }
  } catch (...) {
    std::fill(imageMessage.begin()+5, imageMessage.end(), 255);
    std::cerr << "Paint could not be loaded" << std::endl;
  }
}

void EchoServer::handleClientMessage(uint32_t id, const std::string& message) {
  std::cerr << "Error: Client (id:" << id << ") send a string message" << std::endl;
}

std::vector<uint8_t> EchoServer::fixPaintMessage(const std::vector<uint8_t> &message) {
  
  if (message.size() != 10 || message[0] != 1) {
    throw EchoServerException("Not a valid paint message");
  }
  
  std::vector<uint8_t> forwardedMessage = message;
  const uint16_t brushCenterPosX = std::clamp<uint16_t>(to16Bit(forwardedMessage, 1), 0, canvasWidth-1);
  const uint16_t brushCenterPosY = std::clamp<uint16_t>(to16Bit(forwardedMessage, 3), 0, canvasHeight-1);
  const uint16_t brushSize = std::clamp<uint16_t>(to16Bit(forwardedMessage, 8),1,20);
  forwardedMessage[1] = (brushCenterPosX >> 8) & 0xff;
  forwardedMessage[2] = brushCenterPosX & 0xff;
  forwardedMessage[3] = (brushCenterPosY >> 8) & 0xff;
  forwardedMessage[4] = brushCenterPosY & 0xff;
  forwardedMessage[8] = (brushSize >> 8) & 0xff;
  forwardedMessage[9] = brushSize & 0xff;
  return forwardedMessage;
}

void EchoServer::handleClientMessage(uint32_t id, const std::vector<uint8_t>& message) {
  imageLock.lock();
  paint(message);
  imageLock.unlock();
  try {
    sendMessage(fixPaintMessage(message));
  } catch (const EchoServerException& e) {
    std::cout << "Client id" << id << " is trying to fool us with " << e.what() << std::endl;
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
  
void EchoServer::paint(const std::vector<uint8_t>& message) {
  if (message.size() == 10 && message[0] == 1) {
    const uint16_t brushCenterPosX = to16Bit(message, 1);
    const uint16_t brushCenterPosY = to16Bit(message, 3);
    const uint8_t r = message[5];
    const uint8_t g = message[6];
    const uint8_t b = message[7];
    const uint16_t brushSize = std::clamp<uint16_t>(to16Bit(message, 8),1,20);
    for (int32_t y = 0;y<brushSize;++y) {
      for (int32_t x = 0;x<brushSize;++x) {
        const int32_t posX = brushCenterPosX+x;
        const int32_t posY = brushCenterPosY+y;
        if (posX >= 0 && posX < canvasWidth &&
            posY >= 0 && posY < canvasHeight) {
          const size_t serialPos = size_t((posX + posY * canvasWidth)*4+5);
          imageMessage[serialPos+0] = r;
          imageMessage[serialPos+1] = g;
          imageMessage[serialPos+2] = b;
          imageMessage[serialPos+3] = 255;
        }
      }
    }
  }
}
