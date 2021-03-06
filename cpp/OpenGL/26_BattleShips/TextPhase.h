#pragma once

#include <string>

#include "GamePhase.h"

class TextPhase : public GamePhase {
public:
  TextPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& text="");
  
  virtual ~TextPhase() {}
  
  void setText(const std::string& t);
  
  virtual void init() override;

private:
  std::string text;
  size_t animationStep;
  size_t baseTextWidth;

  virtual void animateInternal(double animationTime) override;
  virtual void drawInternal() override;

};
