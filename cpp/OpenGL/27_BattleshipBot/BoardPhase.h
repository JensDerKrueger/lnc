#pragma once

#include <string>

#include "GamePhase.h"

class BoardPhase : public GamePhase {
public:
  BoardPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui boardSize);

protected:
  Vec2ui boardSize;
};
