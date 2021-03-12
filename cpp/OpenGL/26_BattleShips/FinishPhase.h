#pragma once

#include <string>
#include <optional>

#include "BoardPhase.h"
#include "GameGrid.h"

enum class Verification {
  Ok,
  Invalid,
  Unknown
};

class FinishPhase : public BoardPhase {
public:
  FinishPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui& boardSize);

  void prepare(const GameGrid& myBoard, const GameGrid& otherBoard, const std::string& encOtherBoard, size_t status);
  bool getTerminate() const {return terminate;}
  
private:
  GameGrid myBoard{boardSize};
  GameGrid otherBoard{boardSize};
  
  std::string title;
  std::string homeTitle;
  std::string guestTitle;
  
  std::string encOtherBoard;
  
  Verification verification{Verification::Unknown};
  bool terminate;
  
  void drawBoard(const GameGrid& board, Mat4 boardTrans);
  
  virtual void mouseButtonInternal(int button, int state, int mods, double xPosition, double yPosition) override;
  virtual void drawInternal() override;
  virtual void animateInternal(double animationTime) override;
};
