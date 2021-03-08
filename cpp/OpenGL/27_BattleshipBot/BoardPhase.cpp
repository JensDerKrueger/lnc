#include "BattleShips.h"

#include "BoardPhase.h"

BoardPhase::BoardPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui boardSize) :
GamePhase(app,gamePhaseID),
boardSize(boardSize)
{}
