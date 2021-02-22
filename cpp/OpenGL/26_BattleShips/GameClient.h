#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <queue>

#include <Client.h>

#include "../25_GenericGameServer/NetGame.h"

class GameClient : public Client {
public:
  GameClient(const std::string& address, short port, const std::string& name, uint32_t level);

  virtual void handleNewConnection() override;
  virtual void handleServerMessage(const std::string& message) override;

  bool getInitMessageSend() const {
    return initMessageSend;
  }
  
  bool getReceivedPairingInfo() const {
    return receivedPairingInfo;
  }
  
private:
  std::string name{""};
  uint32_t level{0};
  
  bool initMessageSend{false};
  bool receivedPairingInfo{false};
};
