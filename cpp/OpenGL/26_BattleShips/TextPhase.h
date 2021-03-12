#pragma once

#include <string>

#include "GamePhase.h"

class TextPhase : public GamePhase {
public:
  TextPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& title="", const std::string& subtitle="");
  
  virtual ~TextPhase() {}
  
  void setTitle(const std::string& text);
  void setSubtitle(const std::string& text);
  
  virtual void init() override;

private:
  std::string title;
  std::string subtitle;
  size_t animationStep;

  virtual void animateInternal(double animationTime) override;
  virtual void drawInternal() override;

};
