#pragma once

#include <array>
#include <vector>
#include <map>
#include <deque>

#include <Server.h>

class FrontendServer : public Server<SizedClientConnection> {
public:
  FrontendServer(uint16_t port);
  virtual void handleClientMessage(uint32_t id, const std::string& message) override {}
  void newInput(const std::string& name, const std::string& text);
};

class ChatTrisServer : public Server<HttpClientConnection> {
public:
  ChatTrisServer();
  virtual ~ChatTrisServer();
  virtual void handleClientMessage(uint32_t id, const std::string& message) override;

private:
  FrontendServer frontendConnections{11006};
    
  std::pair<std::string,std::string> parseParameter(const std::string& param) const;
  std::map<std::string,std::string> parseParameters(const std::string& params) const;
};
