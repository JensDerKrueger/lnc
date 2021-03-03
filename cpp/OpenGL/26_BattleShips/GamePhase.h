#pragma once

#include <Vec2.h>
#include <FontRenderer.h>

class BattleShips;

enum class GamePhaseID {
  Boot,
  AdressSetup,
  NameSetup,
  Connecting,
  Pairing,
  BoardSetup,
  WaitingBoardSetup,
  MainPhase,
  Finished
};

class GamePhase {
public:
  GamePhase(BattleShips* app, GamePhaseID gamePhaseID) : app(app), gamePhaseID(gamePhaseID) {}
  virtual ~GamePhase() {}
  
  virtual void init() {}
  virtual void mouseMove(double xPosition, double yPosition);
  virtual void mouseButton(int button, int state, int mods,
                           double xPosition, double yPosition) {}
  virtual void keyboardChar(unsigned int codepoint) {}
  virtual void keyboard(int key, int scancode, int action, int mods) {}
  virtual void animate(double animationTime) {}
  virtual void draw();

  GamePhaseID getGamePhaseID() const {return gamePhaseID;}
  
protected:
  BattleShips* app;
  GamePhaseID gamePhaseID;
  Vec2 normPos;

};
