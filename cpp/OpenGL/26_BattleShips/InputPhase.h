#pragma once

#include <string>
#include <optional>

#include "GamePhase.h"

class InputPhase : public GamePhase {
public:
  InputPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& prompt="");
  
  void setPrompt(const std::string& p) {
    prompt = p;
  }
  
  virtual void keyboard(int key, int scancode, int action, int mods) override;
  virtual void keyboardChar(unsigned int codepoint) override;
  virtual void draw() override;
  
  std::optional<std::string> getInput() const {
    if (complete)
      return userInput;
    else
      return {};
  }
  
private:
  bool complete{false};
  std::string prompt;
  std::string userInput;

};
