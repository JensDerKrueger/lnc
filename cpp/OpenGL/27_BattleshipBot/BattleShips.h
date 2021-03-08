#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <array>
#include <optional>
#include <map>
#include <mutex>

#include <FontRenderer.h>

#include "GameClient.h"

#include "GamePhase.h"
#include "TextPhase.h"
#include "InputPhase.h"
#include "BoardSetupPhase.h"
#include "MainPhase.h"
#include "FinishPhase.h"

#include "GameGrid.h"

const Vec2ui boardSize{15,15};

class BattleShips {
public:
  BattleShips();
  virtual ~BattleShips();
      
  void run();
    
  std::shared_ptr<GameClient> getClient() {return client;}
  
  std::string getOtherName() const {return otherName;}
  uint32_t getOtherLevel() const {return otherLevel;}
  
  uint32_t winns{0};
  uint32_t losses{0};
  uint32_t cheaters{0};
  
  void rememberNastyPos(const GameGrid& fullOtherBoard);
  Vec2ui getRandomNastyPos() const;
    
private:
  std::vector<std::string> otherCallsigns;
  
  std::shared_ptr<GameClient> client{nullptr};

  bool addressComplete{false};
  bool nameComplete{false};
  std::string serverAddress{""};
  std::string otherName{""};
  uint32_t otherLevel{0};

  uint32_t level{1};
  
  std::string myPassword;
  std::string encOtherShipPlacement;
    
  GamePhase* currentPhase{nullptr};
  InputPhase adressPhase;
  TextPhase connectingPhase;
  TextPhase pairingPhase;
  BoardSetupPhase boardSetupPhase;
  TextPhase waitingBoardSetupPhase;
  MainPhase mainPhase;
  FinishPhase finishedPhase;

  void stateTransition();
  void tryToLoadSettings();
  void restartGame(bool reconnect);
  std::string limitString(const std::string& str, size_t maxSize) const;
  
  std::string generateName() const;
  void rememberName(const std::string& str);
  
  std::vector<Vec2ui> nastyPos;

};
