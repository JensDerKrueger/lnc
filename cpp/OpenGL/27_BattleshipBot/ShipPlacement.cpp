#include "ShipPlacement.h"

#include <AES.h>
#include <NetCommon.h>

std::vector<ShipSize> ShipPlacement::completePlacement {
  ShipSize::FIVE,
  ShipSize::FOUR,
  ShipSize::THREE,
  ShipSize::THREE,
  ShipSize::TWO,
};

Ship::Ship(ShipSize shipSize, Orientation orientation, const Vec2ui& pos) :
  shipSize(shipSize),
  orientation(orientation),
  pos(pos)
{
}

bool Ship::check(const ShipLocation& loc) const{
  return loc.start == pos && loc.end == computeEnd();
}

Vec2ui Ship::computeEnd() const {
  return (Orientation::Vertical == orientation)
                              ? Vec2ui(pos.x, pos.y+uint32_t(shipSize)-1)
                              : Vec2ui(pos.x+uint32_t(shipSize)-1, pos.y);
}


ShipPlacement::ShipPlacement(const Vec2ui& gridSize) :
  gridSize(gridSize)
{
  buildCompletePlacementMap();
}

bool ShipPlacement::shipTypeValid(const Ship& newShip) const {
  if (ships.size() == completePlacement.size()) return false;

  std::map<ShipSize, size_t> histo;

  histo[newShip.shipSize]++;
  for (const Ship& other : ships) {
    histo[other.shipSize]++;
  }
  
  try {
    for (const auto& e : histo) {
      if (completePlacementMap.at(e.first) < e.second) return false;
    }
  } catch (const std::out_of_range& ){
    return false;
  }
  
  return true;
}

bool ShipPlacement::shipInGrid(const Ship& newShip) const {
  const Vec2ui end   = newShip.computeEnd();
  return end.x < gridSize.x && end.y < gridSize.y;
}

bool ShipPlacement::shipCollisionFree(const Ship& newShip) const {
  const Vec2ui start = newShip.pos;
  const Vec2ui end   = newShip.computeEnd();

  for (const Ship& other : ships) {
    const Vec2ui otherStart = other.pos;
    const Vec2ui otherEnd   = other.computeEnd();
    if (end.x+1 < otherStart.x || start.x > otherEnd.x+1 ||
        end.y+1 < otherStart.y || start.y > otherEnd.y+1) continue;
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

void ShipPlacement::deleteLastShip() {
  ships.pop_back();
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
  StringEncoder e;
  e.add("ShipPlacement");
  e.add(gridSize.x);
  e.add(gridSize.y);
  e.add(uint32_t(ships.size()));
  for (const Ship& s : ships) {
    e.add(uint32_t(s.shipSize));
    e.add(uint32_t(s.orientation));
    e.add(s.pos.x);
    e.add(s.pos.y);
  }
  return e.getEncodedMessage();
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
    if (pos.x >= start.x && pos.x <= end.x &&
        pos.y >= start.y && pos.y <= end.y) return true;
  }
  return false;
}

bool ShipPlacement::isValid() const {
  ShipPlacement temp{gridSize};
  if (ships.size() != completePlacement.size()) return false;
  for (const Ship& s : ships) {
    if (!temp.addShip(s)) return false;
  }
  return true;
}

void ShipPlacement::buildCompletePlacementMap() {
  completePlacementMap.clear();
  for (const ShipSize& s : completePlacement) {
    completePlacementMap[s]++;
  }
}

size_t ShipPlacement::getHitsToWin() {
  size_t total = 0;
  for (const ShipSize& s : completePlacement) {
    total += size_t(s);
  }
  return total;
}

size_t ShipPlacement::findShip(const ShipLocation& loc) const {
  for (size_t i = 0;i<ships.size();++i) {
    if (ships[i].check(loc)) {
      return i;
    }
  }
  return ShipPlacement::completePlacement.size();
}
