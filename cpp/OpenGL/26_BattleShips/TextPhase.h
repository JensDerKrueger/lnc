#pragma once

#include <string>

#include "GamePhase.h"

class TextPhase : public GamePhase {
public:
  TextPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& text="");
  
  virtual ~TextPhase() {}
  
  void setText(const std::string& t);
  
  virtual void init() override;
  virtual void animate(double animationTime) override;
  virtual void draw() override;
  
private:
  std::string text;
  size_t animationStep;
  size_t baseTextWidth;

};
