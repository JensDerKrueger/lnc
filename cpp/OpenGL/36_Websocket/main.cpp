#include <iostream>
#include <thread>
#include <Server.h>

class EchoServer : public Server<WebSocketConnection> {
public:
  EchoServer(uint16_t port) :
  Server(port)
  {
  }
  
  virtual ~EchoServer() {
  }
  
  virtual void handleClientMessage(uint32_t id, const std::string& message) override {
    sendMessage(message);
  }

  virtual void handleClientMessage(uint32_t id, const std::vector<uint8_t>& message) override {
    sendMessage(message);
  }

  virtual void handleClientConnection(uint32_t id, const std::string& address, uint16_t port) override {
    const uint16_t canvasWidth = 500;
    const uint16_t canvasHeight = 500;
    
    std::cout << "New client (id:" << id << ") connected from " << address << std::endl;
    
    std::vector<uint8_t> message(canvasWidth*canvasHeight*4+1);
    message[0] = 0;
    for (size_t i = 0;i<canvasWidth*canvasHeight;++i) {
      message[1+i*4+0] = 200;
      message[1+i*4+1] = 255;
      message[1+i*4+2] = 200;
      message[1+i*4+3] = 255;
    }
    
    sendMessage(message,id);
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

