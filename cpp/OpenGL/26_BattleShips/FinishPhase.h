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
  
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override;
  virtual void draw() override;
  virtual void animate(double animationTime) override;

  bool getTerminate() const {return terminate;}
private:
  GameGrid myBoard{boardSize};
  GameGrid otherBoard{boardSize};
  
  std::string encOtherBoard;
  
  Verification verification{Verification::Unknown};
  bool terminate;
  std::string guestTitle;
  std::string homeTitle;
  std::string title;
  
  void drawBoard(const GameGrid& board, Mat4 boardTrans);
};
