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

void GameClient::parseGameMessage(const std::string& m) {
  Tokenizer tokenizer{m, char(2)};

  GameMessageType mt = GameMessageType(tokenizer.nextUint32());
  switch (mt) {
    case GameMessageType::EncryptedShipPlacement :
      otherShipPlacement = tokenizer.nextString();
      break;
    case GameMessageType::Aim : {
        uint32_t x = tokenizer.nextUint32();
        uint32_t y = tokenizer.nextUint32();
        const std::scoped_lock<std::mutex> lock(aimMutex);
        aim = Vec2ui{x,y};
      }
      break;
    case GameMessageType::Shot : {
        uint32_t x = tokenizer.nextUint32();
        uint32_t y = tokenizer.nextUint32();
        const std::scoped_lock<std::mutex> lock(shotReceivedMutex);
        shotReceived.push_back({x,y});
      }
      break;
    case GameMessageType::ShotResult : {
        const std::scoped_lock<std::mutex> lock(shotResultMutex);
        shotResult.push_back(tokenizer.nextUint32());
      }
      break;
    case GameMessageType::ShipPlacementPassword :
      shipPlacementPassword = tokenizer.nextString();
      break;
    default:
      std::cerr << "Unkown game message received" << std::endl;
      break;
  }
}

void GameClient::lostConnection() {
  otherShipPlacement = {};
  receivedPairingInfo = false;
}

void GameClient::handleServerMessage(const std::string& message) {
  MessageType pt = identifyString(message);
     
  try {
    switch (pt) {
      case MessageType::PairedMessage : {
        receivedPairingInfo = true;
        break;
      }
      case MessageType::GameMessage : {
        GameMessage m{message};
        parseGameMessage(m.payload);
        break;
      }
      case MessageType::LostUserMessage : {
        lostConnection();
        break;
      }
      default:
        std::cerr << "Unkown network message received" << std::endl;
        break;
    }
  } catch (const MessageException& e) {
    std::cerr << "MessageException: " << e.what() << std::endl;
  }
}

std::optional<std::string> GameClient::getEncryptedShipPlacement() const {
  return otherShipPlacement;
}

void GameClient::sendEncryptedShipPlacement(const std::string& sp) {
  sendGameMessage(GameMessageType::EncryptedShipPlacement, {sp});
  shotReceived.clear();
  shotResult.clear();
}

void GameClient::sendGameMessage(GameMessageType mt, const std::vector<std::string>& data) {
  Encoder e{char(2)};
  e.add(uint32_t(mt));
  e.add(data);
  GameMessage m;
  m.payload = e.getEncodedMessage();
  sendMessage(m.toString());
}

std::optional<std::string> GameClient::getShipPlacementPassword() const {
  return shipPlacementPassword;
}

void GameClient::sendShipPlacementPassword(const std::string& sp) {
  sendGameMessage(GameMessageType::ShipPlacementPassword, {sp});
}

std::vector<Vec2ui> GameClient::getShotReceived() {
  const std::scoped_lock<std::mutex> lock(shotReceivedMutex);
  std::vector<Vec2ui> result = shotReceived;
  return result;
}

std::vector<bool> GameClient::getShotResults() {
  const std::scoped_lock<std::mutex> lock(shotResultMutex);
  std::vector<bool> result = shotResult;
  return result;
}

Vec2ui GameClient::getAim() {
  const std::scoped_lock<std::mutex> lock(aimMutex);
  Vec2ui result = aim;
  return result;
}

void GameClient::fireAt(const Vec2ui& pos) {
  Encoder e{char(2)};
  e.add(uint32_t(GameMessageType::Shot));
  e.add(pos.x());
  e.add(pos.y());
  GameMessage m;
  m.payload = e.getEncodedMessage();
  sendMessage(m.toString());
}

void GameClient::aimAt(const Vec2ui& pos) {
  if (lastAim == pos) return;
  lastAim = pos;
  
  Encoder e{char(2)};
  e.add(uint32_t(GameMessageType::Aim));
  e.add(pos.x());
  e.add(pos.y());
  GameMessage m;
  m.payload = e.getEncodedMessage();
  sendMessage(m.toString());
}

