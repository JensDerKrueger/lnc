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
    case GameMessageType::Shot :
      // TODO
      break;
    case GameMessageType::ShotResult :
      // TODO
      break;
    case GameMessageType::ShipPlacementPassword :
      // TODO
      break;
    default:
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
        std::cerr << "Unkown Message received" << std::endl;
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
}

void GameClient::sendGameMessage(GameMessageType mt, const std::vector<std::string>& data) {
  Encoder e{char(2)};
  e.add(uint32_t(mt));
  e.add(data);
  GameMessage m;
  m.payload = e.getEncodedMessage();
  sendMessage(m.toString());
}
