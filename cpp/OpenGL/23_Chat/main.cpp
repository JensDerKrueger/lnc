#include <iostream>
#include "Server.h"
#include "Client.h"

class MyServer : public Server {
public:
  MyServer(short port) : Server(port) {}
  
  virtual void handleClientMessage(size_t id, const std::string& message) override {
    std::cout << "Client " << id << " send message " << message << std::endl;
    sendMessage(message, id, true);
  }
};


class MyClient : public Client {
public:
  MyClient(const std::string& address, short port) : Client(address, port, 5000) {}
  
  virtual void handleServerMessage(const std::string& message) override {
    std::cout << "Server: " << message << std::endl;
  }
};


int main(int argc, char ** argv) {
  
  if (argc == 2) {
    MyClient c{argv[1],11000};
    std::cout << "connecting ...";
    while (c.isConnecting()) {
      std::cout << "." << std::flush;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << std::endl;
    if (c.isOK()) {
      c.sendMessage("Hallo Leute!");
      std::string message;
      while (true) {
        std::getline (std::cin,message);
        if (message == "q") {
          c.sendMessage("Bye Bye!");
          break;
        } else {
          
          for (size_t i = 0;i<100;++i)
            c.sendMessage(message);
        }
      }
      return EXIT_SUCCESS;
    } else {
      std::cout << "Unable to start client" << std::endl;
      return EXIT_FAILURE;
    }
  } else {
    MyServer s{11000};
    std::cout << "starting ...";
    while (s.isStarting()) {
      std::cout << "." << std::flush;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << std::endl;
    
    if (s.isOK()) {
      std::cout << "running ..." << std::endl;
      std::string test;
      std::cin >> test;
      return EXIT_SUCCESS;
    } else {
      std::cout << "Unable to start server" << std::endl;
      return EXIT_FAILURE;
    }
  }
}
