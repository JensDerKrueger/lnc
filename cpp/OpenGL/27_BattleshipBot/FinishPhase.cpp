#include "BattleShips.h"

#include "FinishPhase.h"

FinishPhase::FinishPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui& b) :
BoardPhase(app, gamePhaseID, b)
{
}

void FinishPhase::run() {
  const auto password = app->getClient()->getShipPlacementPassword();
  if (password) {
    if (otherBoard.validate(encOtherBoard, *password)) {
      cheater = false;
      if (!won) {
        const ShipPlacement sp = ShipPlacement::fromEncryptedString(encOtherBoard, *password);
        otherBoard.setShips(sp);
        app->rememberNastyPos(otherBoard);
      }
    } else {
      cheater = true;
    }
    terminate = true;
  }
}

void FinishPhase::prepare(const GameGrid& my, const GameGrid& other, const std::string& enc, size_t status) {  
  verification = Verification::Unknown;
  terminate = false;
  won = (status == 1);
  
  myBoard = my;
  otherBoard = other;
  encOtherBoard = enc;  
}

