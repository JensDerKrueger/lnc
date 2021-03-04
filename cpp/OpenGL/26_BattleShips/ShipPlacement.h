#pragma once
#include <vector>
#include <map>

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
  void deleteLastShip();
  
  const std::vector<Ship>& getShips() const {return ships;}

  std::string toEncryptedString(const std::string& password) const;
  static ShipPlacement fromEncryptedString(const std::string& encryptedString, const std::string& password);

  bool incomming(const Vec2ui& pos) const;
  
  bool isValid() const;
  
  Vec2ui getGridSize() const {return gridSize;}

  static std::vector<ShipSize> completePlacement;
  
  static size_t getHitsToWin();

private:
  Vec2ui gridSize;
  std::vector<Ship> ships;
  std::map<ShipSize, size_t> completePlacementMap;

  bool shipTypeValid(const Ship& newShip) const;
  bool shipInGrid(const Ship& newShip) const;
  bool shipCollisionFree(const Ship& newShip) const;

  std::string toString() const;
  ShipPlacement(const std::string& str);

  void buildCompletePlacementMap();
};
