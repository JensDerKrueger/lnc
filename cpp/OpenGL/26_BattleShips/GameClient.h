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
  GameClient(const std::string& address, short port, const std::string& name);

  void initDataFromServer();
  virtual void handleNewConnection() override;
  virtual void handleServerMessage(const std::string& message) override;

  bool isValid() const;

private:
  std::string name{""};
  bool initComplete{false};
};
