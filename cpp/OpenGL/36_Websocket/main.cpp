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
    std::cout << "Str: " << message << std::endl;
    sendMessage(message, id);
  }

  virtual void handleClientMessage(uint32_t id, const std::vector<uint8_t>& message) override {
    std::cout << "Bin: [" << std::hex;
    for (size_t i = 0;i<message.size();++i) {
      std::cout << "0x" << int(message[i]) << ((i<message.size()-1) ? ", " : "");
    }
    std::cout << "]" << std::dec << std::endl;
    sendMessage(message, id);
  }


};


int main(int argc, char ** argv) {
  EchoServer s(8899);
  s.start();
  
  while (true) {
  }
  
  return EXIT_SUCCESS;
}

