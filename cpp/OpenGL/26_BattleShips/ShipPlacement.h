#pragma once
#include <vector>

#include <Vec2.h>

enum class ShipSize {
  TWO = 2,
  THREE = 3,
  FOUR = 4,
  FIVE = 5
};

enum class Orientation {
  Horizontal = 0,
  Vertical = 1,
};

class Ship {
public:
  Ship(ShipSize shipSize, Orientation orientation, const Vec2ui& pos);
  
  ShipSize shipSize;
  Orientation orientation;
  Vec2ui pos;
  
  Vec2ui computeEnd() const;
  
};

class ShipPlacement {
public:
  ShipPlacement(const Vec2ui& gridSize=Vec2ui{10,10});
  
  bool checkShip(const Ship& ship) const;

  bool addShip(const Ship& shid);
  void deleteShipAt(size_t shipIndex);
  
  const std::vector<Ship>& getShips() const {return ships;}

private:
  Vec2ui gridSize;
  std::vector<Ship> ships;

  bool shipTypeValid(const Ship& newShip) const;
  bool shipInGrid(const Ship& newShip) const;
  bool shipCollisionFree(const Ship& newShip) const;

};
