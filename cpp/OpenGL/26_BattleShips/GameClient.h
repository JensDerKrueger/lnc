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

enum class GameMessageType {
  Invalid = 0,
  EncryptedShipPlacement = 1,
  Aim = 2,
  Shot = 3,
  ShotResult = 4,
  ShipPlacementPassword = 5
};

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

  std::optional<std::string> getShipPlacementPassword() const;
  void sendShipPlacementPassword(const std::string& sp);
  
  std::vector<Vec2ui> getShotReceived();
  std::vector<bool> getShotResults();
  Vec2ui getAim();
  
  void fireAt(const Vec2ui& pos);
  void aimAt(const Vec2ui& pos);
  
private:
  std::string name{""};
  uint32_t level{0};
  std::optional<std::string> otherShipPlacement{};
  std::optional<std::string> shipPlacementPassword{};
  
  std::mutex aimMutex;
  std::mutex shotReceivedMutex;
  std::mutex shotResultMutex;
  Vec2ui aim;
  Vec2ui lastAim;
  std::vector<Vec2ui> shotReceived;
  std::vector<bool> shotResult;
  
  bool initMessageSend{false};
  bool receivedPairingInfo{false};
  
  void parseGameMessage(const std::string& m);
  void lostConnection();
  
  void sendGameMessage(GameMessageType mt, const std::vector<std::string>& data);
};
