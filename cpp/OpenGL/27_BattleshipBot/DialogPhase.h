#pragma once

#include <string>
#include <optional>

#include "GamePhase.h"
#include "GameGrid.h"

class DialogPhase : public GamePhase {
public:
  DialogPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& prompt);
  virtual ~DialogPhase() {}
  virtual void init() override;
  int getAnswer() const {return answer;}
  
private:
  std::string prompt;
  GLTexture2D texture;
  int answer;
    
  virtual void drawInternal() override;
  virtual void keyboardInternal(int key, int scancode, int action, int mods) override;

};
