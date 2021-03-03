#include <iostream>
#include <sstream>
#include <memory>
#include <limits>
#include <map>
#include <vector>
#include <chrono>

#include <Server.h>
#include <NetCommon.h>

#include "NetGame.h"

class ClientInfo {
public:
  uint32_t id{0};
  std::string name{""};
  GameIDs gameID{GameIDs::InvalidID};
  uint32_t level{0};
  bool paired{false};
  uint32_t partnerID{0};
  
  ClientInfo() {}
  
  ClientInfo(uint32_t id, const std::string& name, GameIDs gameID, uint32_t level) :
    id{id},
    name{cleanupName(name)},
    gameID{gameID},
    level{level},
    paired{false},
    partnerID{0}
  {}

  virtual ~ClientInfo() {}
  
  static std::string cleanupName(const std::string& name) {
    std::string cName{name};
    for (size_t i = 0;i<name.length();++i) {
      cName[i] = cleanupChar(name[i]);
    }
    return cName;
  }

private:
  static char cleanupChar(char c) {
    std::string validChars{"01234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ(),._ ;:"};
    if (validChars.find(c) != std::string::npos) return c; else return '_';
  }

};

typedef std::chrono::high_resolution_clock Clock;

class GGS : public Server {
public:
  bool showErrors{false};
  
  GGS(short port) :
    Server(port)
  {
  }
  
  virtual ~GGS() {
  }
  
  virtual void handleClientConnection(uint32_t id) override {
  }
  
  virtual void handleClientDisconnection(uint32_t id) override {
    ciMutex.lock();

    const uint32_t partnerID = clientInfos[id].partnerID;    
    if (partnerID != 0) {
      LostUserMessage l;
      l.userID = id;
      sendMessage(l.toString(), partnerID);
      clientInfos[partnerID].partnerID = 0;
    }
    clientInfos.erase(id);
    searchForMatch(partnerID);
    ciMutex.unlock();
  }
  
  void pair(uint32_t a, uint32_t b) {
    clientInfos[a].partnerID = b;
    clientInfos[b].partnerID = a;

    PairedMessage l;
    l.userID = a;
    sendMessage(l.toString(), b);

    l.userID = b;
    sendMessage(l.toString(), a);
  }
  
  void searchForMatch(uint32_t id) {
    if (clientInfos.size() < 2) return;
    
    uint32_t match{0};
    uint32_t minAbs{std::numeric_limits<uint32_t>::max()};
    for (const auto& client : clientInfos) {
      if (client.second.id != id &&
          client.second.gameID == clientInfos[id].gameID &&
          client.second.partnerID == 0 &&
          uint32_t(abs(int64_t(client.second.level) - int64_t(clientInfos[id].level))) < minAbs) {
        match = client.second.id;
        minAbs = uint32_t(abs(int64_t(client.second.level) - int64_t(clientInfos[id].level)));
      }
    }
    if (match == 0) return;
    
    pair(id, match);
  }
  
  virtual void handleClientMessage(uint32_t id, const std::string& message) override {
    ciMutex.lock();
    try {
      MessageType pt = identifyString(message);
      switch (pt) {
        case MessageType::ConnectMessage : {
          ConnectMessage r(message);
          r.userID = id;
          ClientInfo ci{id, r.name, r.gameID, r.level};
          clientInfos[id] = ci;
          searchForMatch(id);
          break;
        }
        case MessageType::GameMessage : {
          GameMessage g(message);
          g.userID = id;
          sendMessage(g.toString(), clientInfos[id].partnerID);
          break;
        }
        default:
          break;
      };
    } catch (const MessageException& e) {
      if (showErrors) {
        std::cerr << "MessageException: " << e.what() << std::endl;
      }
    }
 
    ciMutex.unlock();
  }
  
  std::vector<ClientInfo> getClientInfo() {
    std::vector<ClientInfo> result;
    ciMutex.lock();
    for (const auto& client : clientInfos) {
      result.push_back(client.second);
    }
    ciMutex.unlock();
    return result;
  }
    
  int64_t uptime() const {
    auto currentTime = Clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(currentTime-startTime).count();
  }

  
private:
  std::map<uint32_t, ClientInfo> clientInfos;
  std::mutex ciMutex;
  
  std::chrono::time_point<Clock> lastime = Clock::now();
  std::chrono::time_point<Clock> startTime = Clock::now();
};

static void globalExceptionHandler () {
  std::cerr << "terminate called after throwing an instance of ";
  try
  {
      std::rethrow_exception(std::current_exception());
  } catch (const std::exception &ex) {
      std::cerr << typeid(ex).name() << std::endl;
      std::cerr << "  what(): " << ex.what() << std::endl;
  } catch (...) {
      std::cerr << typeid(std::current_exception()).name() << std::endl;
      std::cerr << " ...something, not an exception, dunno what." << std::endl;
  }
#ifdef _WIN32
  std::cerr << "errno: " << errno << std::endl;
#else
  std::cerr << "errno: " << errno << ": " << std::strerror(errno) << std::endl;
#endif // _WIN32
  std::abort();
}

int main(int argc, char ** argv) {
  std::set_terminate (globalExceptionHandler);
    
  GGS s{serverPort};
  s.start();
  std::cout << "starting ...";
  while (s.isStarting()) {
    std::cout << "." << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::cout << std::endl;
  if (s.isOK()) {
    std::cout << "running ..." << std::endl;
    std::string user{""};
    do {
      std::cout << "(? for help) >";
      std::cin >> user;
      
      switch (user[0]) {
        case '?' :
          std::cout << "? : help\n";
          std::cout << "l : list active users\n";
          std::cout << "s : statisics\n";
          std::cout << "e : toggle message error display\n";
          std::cout << "q : quit server\n";
          break;
        case 'l' : {
          const std::vector<ClientInfo> ci = s.getClientInfo();
          for (const ClientInfo& client : ci) {
            std::cout << "ID:" << client.id << " Name:" << client.name << " Game ID:" << uint32_t(client.gameID) << " Level:" << client.level << "\n";
          }
          break;
        }
        case 's' : {
          std::cout << "Server Uptime: "<< s.uptime() << " sec.\n";
          break;
        }
        case 'e' : {
          s.showErrors = !s.showErrors;
          std::cout << "Error Display set to : "<< s.showErrors << "\n";
          break;
        }
      }
      std::cout << std::endl;
      
    } while (user != "q");
    
    std::cout << "Shutting down server ..." << std::endl;

    return EXIT_SUCCESS;
  } else {
    std::cerr << "Unable to start server" << std::endl;
    return EXIT_FAILURE;
  }
}
