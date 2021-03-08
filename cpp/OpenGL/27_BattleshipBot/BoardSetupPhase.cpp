#include "BattleShips.h"

#include "BoardSetupPhase.h"

BoardSetupPhase::BoardSetupPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui& b) :
BoardPhase(app, gamePhaseID, b),
myShipPlacement{b}
{}

ShipPlacement BoardSetupPhase::getPlacement() const {
  return myShipPlacement;
}

void BoardSetupPhase::run() {
  // TODO find smart, random ship placement
  
  const std::vector<ShipSize>& setup = ShipPlacement::completePlacement;
  size_t shipsPlaced{0};

  std::cout << "Placing Ships " << std::flush;
  do {
    myShipPlacement = ShipPlacement{boardSize};
    shipsPlaced = 0;
    for (const ShipSize& s : setup) {
      
      for (size_t i = 0;i<100;++i) {
        Orientation o = Orientation(Rand::rand(0,2));
        uint32_t x = Rand::rand<uint32_t>(0,boardSize.x());
        uint32_t y = Rand::rand<uint32_t>(0,boardSize.y());
        if (myShipPlacement.addShip({s,o,{x,y}})) {
          shipsPlaced++;
          break;
        }
      }
    }
    std::cout << "." << std::flush;
  } while(shipsPlaced != setup.size());
  std::cout << " done" << std::endl;
}

void BoardSetupPhase::prepare() {
  myShipPlacement = ShipPlacement{boardSize};
}


