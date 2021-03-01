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

  std::string toEncryptedString(const std::string& password) const;
  static ShipPlacement fromEncryptedString(const std::string& encryptedString, const std::string& password);

  bool incomming(const Vec2ui& pos) const;
  
private:
  Vec2ui gridSize;
  std::vector<Ship> ships;

  bool shipTypeValid(const Ship& newShip) const;
  bool shipInGrid(const Ship& newShip) const;
  bool shipCollisionFree(const Ship& newShip) const;

  std::string toString() const;
  ShipPlacement(const std::string& str);
};

enum class Cell {
  Unknown,
  Empty,
  Ship
};

class GameGrid {
public:
  GameGrid(const Vec2ui& gridSize=Vec2ui{10,10});
  
  

  void setShips(const ShipPlacement& sp);
  
  void addHit(const Vec2ui& pos);
  void addMiss(const Vec2ui& pos);

  bool validate(const std::string& encryptedString, const std::string& password);
  
  Vec2ui getSize() const {return gridSize;}
  Cell getCell(uint32_t x, uint32_t y) const;
  
private:
  Vec2ui gridSize;
  std::vector<Cell> grid;
  std::vector<Vec2ui> hits;
  std::vector<Vec2ui> misses;

  void clearEmpty();
  void clearUnknown();
  
  void addShip(const Vec2ui& pos);
  void setCell(uint32_t x, uint32_t y, Cell c);

};
