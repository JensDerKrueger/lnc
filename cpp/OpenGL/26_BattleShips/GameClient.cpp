#include "GameClient.h"

GameClient::GameClient(const std::string& address, short port, const std::string& name) :
  Client{address, port , "", 5000},
  name{name}
{
}

void GameClient::initDataFromServer() {
  initComplete = true;
}
  
void GameClient::handleNewConnection() {
}

void GameClient::handleServerMessage(const std::string& message) {

}

bool GameClient::isValid() const {
  return initComplete;
}
