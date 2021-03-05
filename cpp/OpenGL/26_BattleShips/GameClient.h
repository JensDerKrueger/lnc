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

struct ShotResult {
  uint32_t x;
  uint32_t y;
  bool hit;
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
  
  std::vector<Vec2ui> getShotsReceived();
  void sendShotResult(const ShotResult& r);
  std::vector<ShotResult> getShotResults();
  Vec2ui getAim();
  
  void shootAt(const Vec2ui& pos);
  void aimAt(const Vec2ui& pos);
  
  void readyForNewPlayer();
  
private:
  std::string name{""};
  uint32_t level{0};
  std::optional<std::string> otherShipPlacement{};
  std::optional<std::string> shipPlacementPassword{};
  
  std::mutex aimMutex;
  std::mutex shotsMutex;
  Vec2ui aim{std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()};
  Vec2ui lastAim{std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()};
  std::vector<Vec2ui> shotsReceived;
  std::vector<ShotResult> shotResults;
  std::vector<Vec2ui> shotsFired;

  bool initMessageSend{false};
  bool receivedPairingInfo{false};
  
  void parseGameMessage(const std::string& m);
  void lostConnection();
  
  void sendGameMessage(GameMessageType mt, const std::vector<std::string>& data);
};
