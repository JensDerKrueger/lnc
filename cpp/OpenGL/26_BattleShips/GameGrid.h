#pragma once

#include <vector>
#include <string>

#include <Vec2.h>

#include "ShipPlacement.h"

enum class Cell {
  Unknown,
  Empty,
  Ship,
  EmptyShot,
  ShipShot
};

class GameGrid {
public:
  GameGrid(const Vec2ui& gridSize=Vec2ui{10,10});

  void setShips(const ShipPlacement& sp);
  void setEnemyShips(const ShipPlacement& sp);
  
  void addHit(const Vec2ui& pos, bool sunk);
  void addMiss(const Vec2ui& pos);
  void addShot(const Vec2ui& pos);

  bool shipSunk(const Vec2ui& pos) const;
  
  bool validate(const std::string& encryptedString, const std::string& password);
  
  Vec2ui getSize() const {return gridSize;}
  Cell getCell(uint32_t x, uint32_t y) const;
  
  void clearUnknown();

  size_t getRemainingHits() const;
  
private:
  Vec2ui gridSize;
  std::vector<Cell> grid;
  std::vector<Vec2ui> hits;
  std::vector<Vec2ui> misses;

  void clearEmpty();
  
  void addShip(const Vec2ui& pos);
  void setCell(uint32_t x, uint32_t y, Cell c);
  void markAsSunk(const Vec2ui& pos);
};
