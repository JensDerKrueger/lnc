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
  
  std::optional<std::string> getEncryptedShipPlacement() const;
  void sendEncryptedShipPlacement(const std::string& sp);
  
private:
  std::string name{""};
  uint32_t level{0};
  std::optional<std::string> otherShipPlacement{};
  
  bool initMessageSend{false};
  bool receivedPairingInfo{false};
  
  void parseGameMessage(const std::string& m);
};
