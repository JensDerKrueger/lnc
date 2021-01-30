#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include "Server.h"
#include "Client.h"


struct Coder {
  
  static inline char DELIM = char(1);
  
  static std::string encode(const std::string& name, const std::string& value) {
    return removeZeroes(name) + DELIM + removeZeroes(value);
  }

  static std::pair<std::string, std::string> decode(const std::string& input) {
    const size_t delimPos = input.find(DELIM,0);
    return std::make_pair<std::string, std::string>(input.substr(0,delimPos),input.substr(delimPos+1));
  }

  static std::string removeZeroes(std::string input) {
    size_t pos=0;
    while(pos<input.size()) {
      pos=input.find(DELIM,pos);
      if(pos==std::string::npos) break;
      input.replace(pos,1,"");
    }
    return input;
  }
};

struct ClientInfo {
  size_t id;
  std::string name;
};

class MyServer : public Server {
public:
  
  MyServer(short port) : Server(port, "asdn932lwnmflj23") {}

  virtual void handleClientConnection(size_t id) override {
    clientInfo.push_back(ClientInfo{id,"Unknown"});
  }
  
  virtual void handleClientDisconnection(size_t id) override {
    for (size_t i = 0;i<clientInfo.size();++i) {
      if (clientInfo[i].id == id) {
        clientInfo.erase(clientInfo.begin()+i);
        break;
      }
    }
  }
  
  std::optional<ClientInfo> getClientInfo(size_t id) {
    for (auto& c : clientInfo) {
      if (c.id == id) {
        return c;
      }
    }
    return {};
  }
  
  virtual void handleClientMessage(size_t id, const std::string& message) override {
    
    const std::pair<std::string, std::string> data = Coder::decode(message);
    
    if (data.first == "name") {
      for (auto& c : clientInfo) {
        if (c.id == id) {
          c.name = data.second;
          break;
        }
      }
    } else if (data.first == "message") {
      auto ci = getClientInfo(id);
      if (ci.has_value() ) {
        sendMessage(ci.value().name + " writes " + data.second, id, true);
      }
    } else if (data.first == "disconnect") {
      auto ci = getClientInfo(id);
      if (ci.has_value() ) {
        sendMessage(ci.value().name + " has left the building!", id, true);
      }
    } else {
      std::cerr << "client send unknown command" << std::endl;
    }
    
  }
  
private:
  std::vector<ClientInfo> clientInfo;
};


class MyClient : public Client {
public:
  MyClient(const std::string& address, short port) : Client(address, port, "asdn932lwnmflj23", 5000) {}
  
  virtual void handleServerMessage(const std::string& message) override {
    std::cout << message << std::endl;
  }
};


int main(int argc, char ** argv) {
  if (argc == 3) {
    MyClient c{argv[1],11000};
    std::cout << "connecting ...";
    while (c.isConnecting()) {
      std::cout << "." << std::flush;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << " Done" << std::endl;
    if (c.isOK()) {
      std::cout << "Hello " << argv[2] << std::endl;
      c.sendMessage(Coder::encode("name",argv[2]));
      std::string message;
      while (true) {
        std::getline (std::cin,message);
        if (message == "q") {
          c.sendMessage(Coder::encode("disconnect","bye bye!"));
          break;
        } else {          
          c.sendMessage(Coder::encode("message",message));
        }
      }
      return EXIT_SUCCESS;
    } else {
      std::cerr << "Unable to start client" << std::endl;
      return EXIT_FAILURE;
    }
  } else if (argc == 1) {
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
      std::cerr << "Unable to start server" << std::endl;
      return EXIT_FAILURE;
    }
  } else {
    std::cerr << "invalid usage" << std::endl;
    return EXIT_FAILURE;
  }
}
