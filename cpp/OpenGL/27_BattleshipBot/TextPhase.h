#pragma once

#include <string>

#include "GamePhase.h"

class TextPhase : public GamePhase {
public:
  TextPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& text="");
  
  virtual ~TextPhase() {}
  void setText(const std::string& t);
  
  virtual void run() override;

private:
  std::string text;
  bool shown{false};

};
