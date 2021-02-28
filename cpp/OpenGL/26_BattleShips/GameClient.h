#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <queue>
#include <optional>

#include <Client.h>

#include "../25_GenericGameServer/NetGame.h"
#include "ShipPlacement.h"
#include "MD5.h"

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
  
  std::optional<MD5Sum> getReceivedShipPlacementMD5() const;
  void sendShipPlacementMD5(const MD5Sum& md5);
  
private:
  std::string name{""};
  uint32_t level{0};
  
  bool initMessageSend{false};
  bool receivedPairingInfo{false};
};
