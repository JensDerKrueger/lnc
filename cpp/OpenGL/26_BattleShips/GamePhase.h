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
  Finished,
  QuitDialog
};

class GamePhase {
public:
  GamePhase(BattleShips* app, GamePhaseID gamePhaseID) : app(app), gamePhaseID(gamePhaseID) {}
  virtual ~GamePhase() {}
  
  virtual void init() {}
  virtual void mouseMove(double xPosition, double yPosition);
  virtual void mouseButton(int button, int state, int mods,
                           double xPosition, double yPosition);
  virtual void keyboardChar(unsigned int codepoint);
  virtual void keyboard(int key, int scancode, int action, int mods);
  virtual void animate(double animationTime);
  virtual void draw();

  void setBackground(const Image& image, bool keepAspect=true);

  GamePhaseID getGamePhaseID() const;
  
  void setOverlay(std::shared_ptr<GamePhase> overlayPhase);
  std::shared_ptr<GamePhase> getOverlay() const {return overlayPhase;}
  
protected:
  BattleShips* app;
  GamePhaseID gamePhaseID;
  Vec2 normPos;
  bool keepBackgroundAspect;
  std::shared_ptr<GLTexture2D> backgroundImage{nullptr};

  std::shared_ptr<GamePhase> overlayPhase{nullptr};

  virtual void mouseMoveInternal(double xPosition, double yPosition);
  virtual void mouseButtonInternal(int button, int state, int mods,
                                   double xPosition, double yPosition) {}
  virtual void keyboardCharInternal(unsigned int codepoint) {}
  virtual void keyboardInternal(int key, int scancode, int action, int mods) {}
  virtual void animateInternal(double animationTime) {}
  virtual void drawInternal();

  
};
