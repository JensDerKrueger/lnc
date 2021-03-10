#pragma once

#include <iostream>
#include <memory>
#include <limits>
#include <map>
#include <vector>
#include <chrono>
#include <mutex>

#include <Server.h>
#include <NetCommon.h>

#include "NetGame.h"

typedef std::chrono::high_resolution_clock Clock;

class ClientInfo {
public:
  uint32_t id{0};
  std::string name{""};
  GameIDs gameID{GameIDs::InvalidID};
  uint32_t level{0};
  bool paired{false};
  uint32_t partnerID{0};
  
  ClientInfo();
  ClientInfo(uint32_t id, const std::string& name, GameIDs gameID, uint32_t level);
  virtual ~ClientInfo();
  
  static std::string cleanupName(const std::string& name);

private:
  static char cleanupChar(char c);
};


class GGS : public Server {
public:
  bool showErrors{false};
  
  GGS(short port);
  virtual ~GGS();
  
  virtual void handleClientConnection(uint32_t id) override;
  virtual void handleClientDisconnection(uint32_t id) override;
  virtual void handleClientMessage(uint32_t id, const std::string& message) override;
  
  void pair(uint32_t a, uint32_t b);
  void searchForMatch(uint32_t id);

  std::vector<ClientInfo> getClientInfo();
  int64_t uptime() const;
  
  std::string getLogFile() const;
  void setLogFile(const std::string& filename);

private:
  std::string logFile{""};
  std::map<uint32_t, ClientInfo> clientInfos;
  std::mutex ciMutex;
  mutable std::mutex logMutex;
  
  std::chrono::time_point<Clock> lastime = Clock::now();
  std::chrono::time_point<Clock> startTime = Clock::now();
  
  void writeLog(const std::string& str) const;
};
