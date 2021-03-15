#pragma once

#include <string>

#include "BoardPhase.h"
#include "GameGrid.h"

enum class Status {
  SEARCHING,
  SINKING
};

class MainPhase : public BoardPhase {
public:
  MainPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui& boardSize);

  void prepare(const ShipPlacement& myShipPlacement);

  uint32_t gameOver() const;
  GameGrid getMyBoard() const {return myBoard;}
  GameGrid getOtherBoard() const {return otherBoard;}
  
  virtual void run() override;
  
private:
  GameGrid myBoard{boardSize};
  GameGrid otherBoard{boardSize};
  
  size_t myRound{0};
  size_t otherRound{0};
  std::vector<Vec2ui> shotsFired;
  std::vector<Vec2ui> shotsResponded;
  std::vector<Vec2ui> shotsReceived;
  std::vector<ShotResult> shotResults;

  Vec2ui hitCoords{0,0};
  Status status{Status::SEARCHING};
  float alpha{0};
  Vec2ui lastShot{0,0};
  Vec2ui nextShot{0,0};
  
  Vec2ui findNextAim() const;
  void findNextShot();
  
};
