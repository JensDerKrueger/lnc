#pragma once

#include <string>
#include <optional>

#include "GamePhase.h"

class InputPhase : public GamePhase {
public:
  InputPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& prompt="", const std::string& title="");
  
  virtual void init() override;

  void setPrompt(const std::string& p);
  std::optional<std::string> getInput() const;
  
private:
  bool complete{false};
  std::string prompt;
  std::string userInput;
  std::string title;
  
  GLTexture2D titleTex;

  virtual void keyboardInternal(int key, int scancode, int action, int mods) override;
  virtual void keyboardCharInternal(unsigned int codepoint) override;
  virtual void drawInternal() override;

};
