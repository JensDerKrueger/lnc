#pragma once

#include <string>
#include <optional>

#include "BoardPhase.h"
#include "GameGrid.h"

class MainPhase : public BoardPhase {
public:
  MainPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui& boardSize);

  virtual void mouseMove(double xPosition, double yPosition) override;
  virtual void mouseButton(int button, int state, int mods,
                           double xPosition, double yPosition) override;
  virtual void draw() override;

private:
  Mat4 otherBoardTrans;
  Vec2ui otherCellPos;

  void drawBoard(const GameGrid& board, Mat4 boardTrans, Vec2ui aimCoords);
};
