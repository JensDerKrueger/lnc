#include "ShipPlacement.h"

#include <map>

#include <AES.h>
#include <NetCommon.h>


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

ShipPlacement ShipPlacement::fromEncryptedString(const std::string& encryptedString, const std::string& password) {
  AESCrypt crypt("1234567890123456",password);
  return ShipPlacement(crypt.decryptString(encryptedString));
}

std::string ShipPlacement::toEncryptedString(const std::string& password) const {
  AESCrypt crypt("1234567890123456",password);
  return crypt.encryptString(toString());
}

std::string ShipPlacement::toString() const {
  std::vector<std::string> data;
  data.push_back("ShipPlacement");
  data.push_back(std::to_string(gridSize.x()));
  data.push_back(std::to_string(gridSize.y()));
  data.push_back(std::to_string(ships.size()));
  for (const Ship& s : ships) {
    data.push_back(std::to_string(uint32_t(s.shipSize)));
    data.push_back(std::to_string(uint32_t(s.orientation)));
    data.push_back(std::to_string(s.pos.x()));
    data.push_back(std::to_string(s.pos.y()));
  }
  return Coder::encode(data);
}

ShipPlacement::ShipPlacement(const std::string& str) {
  Tokenizer t{str};
  
  if (t.nextString() != "ShipPlacement") throw MessageException("Invalid ShipPlacement");
  
  const uint32_t w = t.nextUint32();
  const uint32_t h = t.nextUint32();
  
  gridSize = Vec2ui{w,h};
  
  const size_t shipCount = t.nextUint32();
  for (size_t i = 0;i<shipCount;++i) {
    const ShipSize shipSize = ShipSize(t.nextUint32());
    const Orientation orientation = Orientation(t.nextUint32());
    const uint32_t x = t.nextUint32();
    const uint32_t y = t.nextUint32();
    const Vec2ui pos{x,y};
    ships.push_back({shipSize, orientation, pos });
  }
}

bool ShipPlacement::incomming(const Vec2ui& pos) const {
  for (const Ship& s : ships) {
    const Vec2ui start = s.pos;
    const Vec2ui end   = s.computeEnd();    
    if (pos.x() >= start.x() && pos.x() <= end.x() &&
        pos.y() >= start.y() && pos.y() <= end.y()) return true;
  }
  return false;
}

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

void GameGrid::addShip(const Vec2ui& pos) {
  setCell(pos.x(), pos.y(), Cell::Ship);
}

bool GameGrid::validate(const std::string& encryptedString, const std::string& password) {
  try {
    ShipPlacement sp = ShipPlacement::fromEncryptedString(encryptedString, password);
    
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
