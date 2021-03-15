#pragma once

#include <string>

#include "BoardPhase.h"

class BoardSetupPhase : public BoardPhase {
public:
  BoardSetupPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui& boardSize);

  ShipPlacement getPlacement() const;
  void prepare();
  void run() override;
    
private:
  ShipPlacement myShipPlacement;

};
