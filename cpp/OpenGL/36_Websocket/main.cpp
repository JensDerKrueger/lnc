#include <iostream>
#include <vector>
#include <string>
#include <thread>

#include <Server.h>

constexpr const uint16_t canvasWidth = 500;
constexpr const uint16_t canvasHeight = 500;

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
    sendMessage(message);
  }

  virtual void handleClientConnection(uint32_t id, const std::string& address, uint16_t port) override {
    
    std::cout << "New client (id:" << id << ") connected from " << address << std::endl;
    
    imageMessage[0] = 0;
    for (size_t i = 0;i<canvasWidth*canvasHeight;++i) {
      imageMessage[1+i*4+0] = 200;
      imageMessage[1+i*4+1] = 255;
      imageMessage[1+i*4+2] = 200;
      imageMessage[1+i*4+3] = 255;
    }
    
    sendMessage(imageMessage,id);
  }

  virtual void handleClientDisconnection(uint32_t id) override {
    std::cout << "Client (id:" << id << ") disconnected" << std::endl;
  }
  
  virtual void handleProtocolMessage(uint32_t id, uint32_t messageID, const std::vector<uint8_t>& message) override {
    std::cout << "Protocol Message 0x" << std::hex << messageID << std::dec << " form id " << id << std::endl;
  }

  virtual void handleError(const std::string& message) override {
    std::cerr << "Error: " << message << std::endl;
  }
  
private:
  
  std::vector<uint8_t> imageMessage;
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

