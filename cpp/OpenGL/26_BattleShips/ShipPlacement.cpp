#include "ShipPlacement.h"

#import <map>

Ship::Ship(ShipSize shipSize, Orientation orientation, const Vec2ui& pos) :
  shipSize(shipSize),
  orientation(orientation),
  pos(pos)
{
}

Vec2ui Ship::computeEnd() const {
  return (Orientation::Vertical == orientation)
                              ? Vec2ui(pos.x(), pos.y()+uint32_t(shipSize)-1)
                              : Vec2ui(pos.x()+uint32_t(shipSize)-1, pos.y());
}


ShipPlacement::ShipPlacement(const Vec2ui& gridSize) :
  gridSize(gridSize)
{
}


bool ShipPlacement::shipTypeValid(const Ship& newShip) const {
  if (ships.size() == 6) return false;

  std::map<ShipSize, size_t> histo;

  histo[ShipSize::TWO] = 0;
  histo[ShipSize::THREE] = 0;
  histo[ShipSize::FOUR] = 0;
  histo[ShipSize::FIVE] = 0;
  
  histo[newShip.shipSize]++;
  for (const Ship& other : ships) {
    histo[other.shipSize]++;
  }
  
  return (histo[ShipSize::TWO] <= 1 && histo[ShipSize::THREE] <= 2 &&
          histo[ShipSize::FOUR] <= 2 && histo[ShipSize::FIVE] <= 1);
}

bool ShipPlacement::shipInGrid(const Ship& newShip) const {
  const Vec2ui end   = newShip.computeEnd();
  return end.x() < gridSize.x() && end.y() < gridSize.y();
}

bool ShipPlacement::shipCollisionFree(const Ship& newShip) const {
  const Vec2ui start = newShip.pos;
  const Vec2ui end   = newShip.computeEnd();

  for (const Ship& other : ships) {
    const Vec2ui otherStart = other.pos;
    const Vec2ui otherEnd   = other.computeEnd();
    if (end.x()+1 < otherStart.x() || start.x() > otherEnd.x()+1 ||
        end.y()+1 < otherStart.y() || start.y() > otherEnd.y()+1) continue;
    return false;
  }
  
  return true;
}

bool ShipPlacement::checkShip(const Ship& ship) const {
  return shipTypeValid(ship) && shipInGrid(ship) && shipCollisionFree(ship);
}

bool ShipPlacement::addShip(const Ship& ship) {
  if (checkShip(ship)) {
    ships.push_back(ship);
    return true;
  }
  return false;
}

void ShipPlacement::deleteShipAt(size_t shipIndex) {
  ships.erase(ships.begin()+shipIndex);
}
