#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <array>
#include <optional>
#include <mutex>

#include <GLApp.h>
#include <GLTexture2D.h>
#include <FontRenderer.h>

#include "GameClient.h"

#include "GamePhase.h"
#include "TextPhase.h"
#include "InputPhase.h"
#include "BoardSetupPhase.h"
#include "MainPhase.h"
#include "FinishPhase.h"

#include "GameGrid.h"

const Vec2ui boardSize{10,10};

class BattleShips : public GLApp {
public:
  BattleShips();
  virtual ~BattleShips();
  
  virtual void init() override;
  virtual void mouseMove(double xPosition, double yPosition) override;
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override;
  virtual void keyboardChar(unsigned int codepoint) override;
  virtual void keyboard(int key, int scancode, int action, int mods) override;
  virtual void animate(double animationTime) override ;
  virtual void draw() override;

  static FontRenderer fr;

  Dimensions getWindowSize() const {return glEnv.getWindowSize();}
    
  std::shared_ptr<GameClient> getClient() {return client;}
  
  std::string getOtherName() const {return otherName;}
  uint32_t getOtherLevel() const {return otherLevel;}
  
private:
  std::shared_ptr<GameClient> client{nullptr};

  bool addressComplete{false};
  bool nameComplete{false};
  std::string serverAddress{""};
  std::string userName{""};
  std::string otherName{""};
  uint32_t otherLevel{0};

  uint32_t level{0};
  
  std::string myPassword;
  std::string encOtherShipPlacement;
    
  GamePhase* currentPhase{nullptr};
  InputPhase adressPhase;
  InputPhase namePhase;
  TextPhase connectingPhase;
  TextPhase pairingPhase;
  BoardSetupPhase boardSetupPhase;
  TextPhase waitingBoardSetupPhase;
  MainPhase mainPhase;
  FinishPhase finishedPhase;

  void stateTransition();
  void tryToLoadSettings();
  void restartGame(bool reconnect);
};
