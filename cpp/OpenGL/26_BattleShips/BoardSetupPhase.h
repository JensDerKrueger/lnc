#pragma once

#include <string>
#include <optional>

#include "BoardPhase.h"

class BoardSetupPhase : public BoardPhase {
public:
  BoardSetupPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui& boardSize);

  virtual void mouseMove(double xPosition, double yPosition) override;
  virtual void mouseButton(int button, int state, int mods,
                           double xPosition, double yPosition) override;
  virtual void keyboard(int key, int scancode, int action, int mods) override;
  virtual void draw() override;

  void reset();
  
  std::optional<ShipPlacement> getPlacement() const;
    
private:
  Mat4 myBoardTrans;
  Vec2ui myCellPos;

  Orientation currentOrientation{Orientation::Vertical};
  size_t currentPlacement{0};
  std::vector<ShipSize> placementOrder{ShipSize::TWO,ShipSize::THREE,ShipSize::THREE,ShipSize::FOUR,ShipSize::FOUR,ShipSize::FIVE};
  ShipPlacement myShipPlacement;
  
  
  void toggleOrientation();
};
