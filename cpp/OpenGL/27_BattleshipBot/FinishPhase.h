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

  virtual void run() override;
  bool getTerminate() const {return terminate;}
  bool getCheater() const {return cheater;}
  
private:
  GameGrid myBoard{boardSize};
  GameGrid otherBoard{boardSize};
  bool terminate{false};
  bool cheater{false};
  bool won;
  
  std::string encOtherBoard;  
  Verification verification{Verification::Unknown};
};
