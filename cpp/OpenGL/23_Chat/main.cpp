#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include "Server.h"
#include "Client.h"


struct ClientInfo {
  uint32_t id;
  std::string name;
};

class MyServer : public Server {
public:
  
  MyServer(short port) : Server(port, "asdn932lwnmflj23") {}

  virtual void handleClientConnection(uint32_t id) override {
    clientInfo.push_back(ClientInfo{id,"Unknown"});
  }
  
  virtual void handleClientDisconnection(uint32_t id) override {
    for (size_t i = 0;i<clientInfo.size();++i) {
      if (clientInfo[i].id == id) {
        sendMessage(clientInfo[i].name + " has left the building!", id, true);
        clientInfo.erase(clientInfo.begin()+i);
        break;
      }
    }
  }
  
  std::optional<ClientInfo> getClientInfo(uint32_t id) {
    for (auto& c : clientInfo) {
      if (c.id == id) {
        return c;
      }
    }
    return {};
  }
  
  virtual void handleClientMessage(uint32_t id, const std::string& message) override {
    try {
      Tokenizer t{message};
      const std::string messageID = t.nextString();
      const std::string messageContent = t.nextString();
        
      if (messageID == "name") {
        for (auto& c : clientInfo) {
          if (c.id == id) {
            c.name = messageContent;
            break;
          }
        }
        sendMessage(messageContent + " joined the chat!", id, true);
      } else if (messageID == "message") {
        auto ci = getClientInfo(id);
        if (ci.has_value() ) {
          sendMessage(ci.value().name + " writes " + messageContent, id, true);
        }
      }  else {
        std::cerr << "client send unknown command" << std::endl;
      }
    } catch (const MessageException& e) {
      std::cerr << e.what() << std::endl;
    }
    
  }
  
private:
  std::vector<ClientInfo> clientInfo;
};


class MyClient : public Client {
public:
  MyClient(const std::string& address, short port, const std::string& name) :
    Client(address, port, "asdn932lwnmflj23", 5000),
    name(name)
  {}

  std::string clean(const std::string& message) {
    static const std::string safeChars = "abcdefghijklmonqrstuvwxyzABCDEFGHIJKLMNOPRSTUVWXYZ1234567890ß+-*,.;:-_<># !?";

    std::string safeString{""};
    for (size_t i = 0;i<message.size();++i) {
      if ( safeChars.find(message[i]) == std::string::npos ) {
        safeString += "_";
      } else {
        safeString += message[i];
      }
    }
    return safeString;
  }
  
  virtual void handleNewConnection() override {
    Encoder e;
    e.add({"name",name});
    sendMessage(e.getEncodedMessage());
  }

  virtual void handleServerMessage(const std::string& message) override {
    std::cout << clean(message) << std::endl;
  }

private:
  std::string name;
                                
};


int main(int argc, char ** argv) {
  if (argc == 3) {
    MyClient c{argv[1],11000,argv[2]};
    std::cout << "connecting ...";
    while (c.isConnecting()) {
      std::cout << "." << std::flush;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << " Done" << std::endl;
    if (c.isOK()) {
      std::cout << "Hello " << argv[2] << std::endl;
      std::string message;
      while (true) {
        std::getline (std::cin,message);
        if (message == "q") {
          break;
        } else {
          Encoder e;
          e.add({"message",message});
          c.sendMessage(e.getEncodedMessage());
        }
      }
      return EXIT_SUCCESS;
    } else {
      std::cerr << "Unable to start client" << std::endl;
      return EXIT_FAILURE;
    }
  } else if (argc == 1) {
    MyServer s{11000};
    s.start();
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
