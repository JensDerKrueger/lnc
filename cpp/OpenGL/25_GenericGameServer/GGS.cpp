#include "GGS.h"

#include <sstream>
#include <fstream>

typedef std::chrono::high_resolution_clock Clock;


ClientInfo::ClientInfo() {}
  
ClientInfo::ClientInfo(uint32_t id, const std::string& name, GameIDs gameID, uint32_t level) :
  id{id},
  name{cleanupName(name)},
  gameID{gameID},
  level{level},
  paired{false},
  partnerID{0}
{}

ClientInfo::~ClientInfo() {
  
}

std::string ClientInfo::cleanupName(const std::string& name) {
  std::string cName{name};
  for (size_t i = 0;i<name.length();++i) {
    cName[i] = cleanupChar(name[i]);
  }
  return cName;
}

char ClientInfo::cleanupChar(char c) {
  std::string validChars{"01234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ(),._ ;:"};
  if (validChars.find(c) != std::string::npos) return c; else return '_';
}
  
GGS::GGS(uint16_t port) :
  Server(port)
{
}

GGS::~GGS() {
}

void GGS::writeLog(const std::string& str) const {
  const std::scoped_lock<std::mutex> lock(logMutex);

  if (logFile.empty()) return;
  
  std::ofstream log(logFile, std::ios_base::app);
  if (log.is_open()) {
    log << uptime() << " : " << str << std::endl;
    log.close();
  }
}

std::string GGS::getLogFile() const {
  return logFile;
}

void GGS::setLogFile(const std::string& filename) {
  if (!filename.empty()) {
    logFile = filename;
    writeLog("Logging Started");
  } else {
    writeLog("Logging Terminated");
    logFile = filename;
  }
}

void GGS::handleClientConnection(uint32_t id, const std::string& address, uint16_t port) {
  std::stringstream ss;
  ss << "New client " << id << " connected from " << address << ":" << port;
  writeLog(ss.str());
}

void GGS::handleClientDisconnection(uint32_t id) {
  ciMutex.lock();

  std::stringstream ss;
  ss << "Client " << id << " disconnected";
  writeLog(ss.str());

  const uint32_t partnerID = clientInfos[id].partnerID;
  if (partnerID != 0) {
    LostUserMessage l;
    l.userID = id;
    sendMessage(l.toString(), partnerID);
  }
  clientInfos.erase(id);
  ciMutex.unlock();
}

void GGS::handleClientMessage(uint32_t id, const std::string& message) {
  ciMutex.lock();
  try {
    MessageType pt = identifyString(message);
    switch (pt) {
      case MessageType::ConnectMessage : {
        ConnectMessage r(message);
        r.userID = id;
        ClientInfo ci{id, r.name, r.gameID, r.level};
        clientInfos[id] = ci;

        std::stringstream ss;
        ss << "Client " << id << " reports: Name=" << r.name << " Game=" << uint32_t(r.gameID) << " Level=" << r.level;
        writeLog(ss.str());

        searchForMatch(id);
        break;
      }
      case MessageType::ReadyForNewMessage : {
        ReadyForNewMessage r(message);
        clientInfos[id].partnerID = 0;
        
        std::stringstream ss;
        ss << "Client " << id << " reports: Ready for new partner";
        writeLog(ss.str());

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

void GGS::pair(uint32_t a, uint32_t b) {
  clientInfos[a].partnerID = b;
  clientInfos[b].partnerID = a;

  PairedMessage l{clientInfos[a].name, clientInfos[a].level};
  l.userID = a;
  sendMessage(l.toString(), b);

  l.name = clientInfos[b].name;
  l.level = clientInfos[b].level;
  l.userID = b;
  sendMessage(l.toString(), a);
  
  std::stringstream ss;
  ss << "Clients " << a << " and " << b << " are now connected and are playing game " << uint32_t(clientInfos[b].gameID);
  writeLog(ss.str());
}

void GGS::searchForMatch(uint32_t id) {
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


std::vector<ClientInfo> GGS::getClientInfo() {
  std::vector<ClientInfo> result;
  ciMutex.lock();
  for (const auto& client : clientInfos) {
    result.push_back(client.second);
  }
  ciMutex.unlock();
  return result;
}
  
int64_t GGS::uptime() const {
  auto currentTime = Clock::now();
  return std::chrono::duration_cast<std::chrono::seconds>(currentTime-startTime).count();
}
