#pragma once

#include <string>
#include "GamePhase.h"

class InputPhase : public GamePhase {
public:
  InputPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& prompt="");
  
  void setPrompt(const std::string& p) {
    prompt = p;
  }
    
  std::string getInput() const {
    return userInput;
  }
  
  virtual void run() override;
  
private:
  std::string prompt;
  std::string userInput;
};
