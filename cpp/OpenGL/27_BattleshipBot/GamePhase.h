#pragma once

class BattleShips;

enum class GamePhaseID {
  Boot,
  AdressSetup,
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
  virtual void run() = 0;
  GamePhaseID getGamePhaseID() const {return gamePhaseID;}
    
protected:
  BattleShips* app;

private:
  GamePhaseID gamePhaseID;
  
};
