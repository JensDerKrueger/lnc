#include "GameGrid.h"

#include <NetCommon.h>

GameGrid::GameGrid(const Vec2ui& gridSize) :
gridSize{gridSize}
{
  clearUnknown();
}

Cell GameGrid::getCell(uint32_t x, uint32_t y) const {
  if (x>=gridSize.x() || y >=gridSize.y()) return Cell::Unknown;
  return grid[x+y*gridSize.x()];
}

void GameGrid::setCell(uint32_t x, uint32_t y, Cell c) {
  if (x>=gridSize.x() || y >=gridSize.y()) return;  
  grid[x+y*gridSize.x()] = c;
}

void GameGrid::addHit(const Vec2ui& pos) {
  setCell(pos.x(), pos.y(), Cell::Ship);
  hits.push_back(pos);
}

void GameGrid::addMiss(const Vec2ui& pos) {
  setCell(pos.x(), pos.y(), Cell::Empty);
  misses.push_back(pos);
}

void GameGrid::addShot(const Vec2ui& pos) {
  Cell c = getCell(pos.x(), pos.y());
  switch (c) {
    case Cell::Unknown:
    case Cell::ShipShot :
    case Cell::EmptyShot :
      break;
    case Cell::Ship :
      setCell(pos.x(), pos.y(), Cell::ShipShot);
      hits.push_back(pos);
      break;
    case Cell::Empty :
      setCell(pos.x(), pos.y(), Cell::EmptyShot);
      misses.push_back(pos);
      break;
  }
}

void GameGrid::addShip(const Vec2ui& pos) {
  setCell(pos.x(), pos.y(), Cell::Ship);
}

bool GameGrid::validate(const std::string& encryptedString, const std::string& password) {
  try {
    ShipPlacement sp = ShipPlacement::fromEncryptedString(encryptedString, password);
    
    if (!sp.isValid()) return false;
    
    for (const Vec2ui& hit : hits) {
      if (!sp.incomming(hit)) return false;
    }

    for (const Vec2ui& miss : misses) {
      if (sp.incomming(miss)) return false;
    }

  } catch (const MessageException& ) {
    return false;
  }
  return true;
}

void GameGrid::setShips(const ShipPlacement& sp) {
  clearEmpty();
  
  for (const Ship& s : sp.getShips()) {
    const Vec2ui start = s.pos;
    const Vec2ui end   = s.computeEnd();
    
    for (uint32_t y = start.y();y<=end.y();y++) {
      for (uint32_t x = start.x();x<=end.x();x++) {
        addShip({x,y});
      }
    }
  }
}

void GameGrid::setEnemyShips(const ShipPlacement& sp) {
  for (const Ship& s : sp.getShips()) {
    const Vec2ui start = s.pos;
    const Vec2ui end   = s.computeEnd();
    
    for (uint32_t y = start.y();y<=end.y();y++) {
      for (uint32_t x = start.x();x<=end.x();x++) {
        if (getCell(x, y) == Cell::Unknown )
          addShip({x,y});
      }
    }
  }
}

void GameGrid::clearEmpty() {
  grid.resize(gridSize.x() * gridSize.y());
  for (size_t i = 0; i< grid.size();++i) {
    grid[i] = Cell::Empty;
  }
  hits.clear();
  misses.clear();
}

void GameGrid::clearUnknown() {
  grid.resize(gridSize.x() * gridSize.y());
  for (size_t i = 0; i< grid.size();++i) {
    grid[i] = Cell::Unknown;
  }
  hits.clear();
  misses.clear();
}

size_t GameGrid::getRemainingHits() const {
  return ShipPlacement::getHitsToWin() - hits.size();
}

bool GameGrid::shipSunk(const Vec2ui& pos) const {
  
  for (uint32_t x = pos.x()+1;x<gridSize.x();x++) {
    if (getCell(x, pos.y()) == Cell::Ship) return false;
    if (getCell(x, pos.y()) == Cell::Empty || getCell(x, pos.y()) == Cell::EmptyShot) break;
  }
  
  for (int64_t x = int64_t(pos.x())-1;x>0;x--) {
    if (getCell(uint32_t(x), pos.y()) == Cell::Ship) return false;
    if (getCell(uint32_t(x), pos.y()) == Cell::Empty || getCell(uint32_t(x), pos.y()) == Cell::EmptyShot) break;
  }

  for (uint32_t y = pos.y()+1;y<gridSize.y();y++) {
    if (getCell(pos.x(), y) == Cell::Ship) return false;
    if (getCell(pos.x(), y) == Cell::Empty || getCell(pos.x(), y) == Cell::EmptyShot) break;
  }
  
  for (int64_t y = int64_t(pos.y())-1;y>0;y--) {
    if (getCell(pos.x(), uint32_t(y)) == Cell::Ship) return false;
    if (getCell(pos.x(), uint32_t(y)) == Cell::Empty || getCell(pos.x(), uint32_t(y)) == Cell::EmptyShot) break;
  }
  
  return true;
}
