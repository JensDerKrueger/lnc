#include <iostream>
#include <vector>
#include <string>
#include <thread>

#include <Server.h>

constexpr const uint16_t canvasWidth = 500;
constexpr const uint16_t canvasHeight = 500;
constexpr const uint16_t brushWidth = 10;
constexpr const uint16_t brushHeight = 10;


class EchoServer : public Server<WebSocketConnection> {
public:
  EchoServer(uint16_t port) :
  Server(port)
  {
    imageMessage.resize(canvasWidth*canvasHeight*4+1);
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
  
  void paint(const std::vector<uint8_t>& message) {
    if (message.size() == 8 || message[0] == 1) {
      uint16_t brushCenterPosX = uint16_t(uint16_t(message[1]) << 8) | message[2];
      uint16_t brushCenterPosY = uint16_t(uint16_t(message[3]) << 8) | message[4];
      uint8_t r = message[5];
      uint8_t g = message[6];
      uint8_t b = message[7];
            
      for (int32_t y = 0;y<brushHeight;++y) {
        for (int32_t x = 0;x<brushWidth;++x) {
          
          int32_t posX = brushCenterPosX+x;
          int32_t posY = brushCenterPosY+y;
          
          if (posX >= 0 && posX < canvasWidth &&
              posY >= 0 && posY < canvasHeight) {
            
            imageMessage[size_t((posX + posY * canvasWidth)*4+1+0)] = r;
            imageMessage[size_t((posX + posY * canvasWidth)*4+1+1)] = g;
            imageMessage[size_t((posX + posY * canvasWidth)*4+1+2)] = b;
            imageMessage[size_t((posX + posY * canvasWidth)*4+1+3)] = 255;
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

