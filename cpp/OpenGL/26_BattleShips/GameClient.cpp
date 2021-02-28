#include "GameClient.h"

GameClient::GameClient(const std::string& address, short port, const std::string& name, uint32_t level) :
  Client{address, port , "", 5000},
  name{name},
  level{level}
{
}

void GameClient::handleNewConnection() {
  ConnectMessage m{name, GameIDs::BattleShips, level};
  sendMessage(m.toString());
  initMessageSend = true;
}

void GameClient::handleServerMessage(const std::string& message) {
  // TODO
}


std::optional<std::array<uint8_t,16>> GameClient::getReceivedShipPlacementMD5() const {
  // TODO
  return {};
}

void GameClient::sendShipPlacementMD5(const MD5Sum& md5) {
  // TODO
}
