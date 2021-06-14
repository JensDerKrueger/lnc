#include <iostream>
#include <charconv>
#include <array>

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
    std::cout << "New client (id:" << id << ") connected from " << address << std::endl;
  }

  virtual void handleClientDisconnection(uint32_t id) override {
    std::cout << "Client (id:" << id << ") disconnected" << std::endl;
  }
  
  virtual void handleProtocolMessage(uint32_t id, uint32_t messageID, const std::vector<uint8_t>& message) override {
    std::cout << "Protocol Message 0x" << std::hex << messageID << std::dec << " form id " << id << std::endl;
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

