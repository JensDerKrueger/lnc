#include <iostream>
#include <vector>
#include <string>
#include <thread>

#include <Server.h>

constexpr const uint16_t canvasWidth  = 1000;
constexpr const uint16_t canvasHeight = 1000;

class EchoServer : public Server<WebSocketConnection> {
public:
  EchoServer(uint16_t port) :
  Server(port)
  {
    imageMessage.resize(canvasWidth*canvasHeight*4+5);
    imageMessage[0] = 0;
    imageMessage[1] = (canvasWidth >> 8) & 0xff;
    imageMessage[2] = canvasWidth & 0xff;
    imageMessage[3] = (canvasHeight >> 8) & 0xff;
    imageMessage[4] = canvasHeight & 0xff;
    
    std::fill(imageMessage.begin()+5, imageMessage.end(), 255);
  }
  
  virtual ~EchoServer() {
  }
  
  virtual void handleClientMessage(uint32_t id, const std::string& message) override {
    std::cerr << "Error: Client (id:" << id << ") send a string message" << std::endl;
  }

  virtual void handleClientMessage(uint32_t id, const std::vector<uint8_t>& message) override {
    paint(message);
    sendMessage(message);
  }

  virtual void handleClientConnection(uint32_t id, const std::string& address, uint16_t port) override {
    std::cout << "New client (id:" << id << ") connected from " << address << std::endl;
    printStats();
    sendMessage(imageMessage,id);
  }

  virtual void handleClientDisconnection(uint32_t id) override {
    std::cout << "Client (id:" << id << ") disconnected" << std::endl;
    printStats();
  }
  
  virtual void handleProtocolMessage(uint32_t id, uint32_t messageID, const std::vector<uint8_t>& message) override {
    std::cout << "Protocol Message 0x" << std::hex << messageID << std::dec << " form id " << id << std::endl;
  }

  virtual void printStats() {
    std::cout << "A total of " << getValidIDs().size() << " clients are currently connected." << std::endl;
  }
  
  virtual void handleError(const std::string& message) override {
    std::cerr << "Error: " << message << std::endl;
  }
  
private:
  std::vector<uint8_t> imageMessage;
  
  static uint16_t to16Bit(const std::vector<uint8_t>& message, size_t index) {
    return uint16_t(uint16_t(message[index]) << 8) | message[index+1];
  }
  
  void paint(const std::vector<uint8_t>& message) {
    if (message.size() == 10 || message[0] == 1) {
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
};


int main(int argc, char ** argv) {
  EchoServer s(2000);
  s.start();

  std::cout << "Starting ";
  while (s.isStarting()) {
    std::cout << "." << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  std::cout << "\nRunning" << std::endl;;
  while (s.isOK()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  
  return EXIT_SUCCESS;
}

