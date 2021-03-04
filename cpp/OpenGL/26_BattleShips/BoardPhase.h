#pragma once

#include <string>
#include <optional>

#include "GamePhase.h"

class BoardPhase : public GamePhase {
public:
  BoardPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui boardSize);

  virtual void init() override;

protected:
  Vec2ui boardSize;
  GLTexture2D emptyCell;
  GLTexture2D unknownCell;
  GLTexture2D shipCell;
  GLTexture2D aimCell;
  std::vector<float> gridLines;

private:
  std::vector<float> gridToLines() const;
};
