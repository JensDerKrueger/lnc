#include "BattleShips.h"

#include "GamePhase.h"
#include "DialogPhase.h"

void GamePhase::mouseMoveInternal(double xPosition, double yPosition) {
  Dimensions s = app->getWindowSize();
  if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;

  normPos = Vec2{float(xPosition/s.width)-0.5f,float(1.0-yPosition/s.height)-0.5f} * 2.0f;
}

void GamePhase::drawInternal() {
  GL(glEnable(GL_BLEND));
  GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL(glBlendEquation(GL_FUNC_ADD));
  GL(glClearColor(0.0f,0.0f,0.0f,0.0f));
  GL(glClear(GL_COLOR_BUFFER_BIT));

  if (backgroundImage) {
    if (keepBackgroundAspect)
      app->setDrawTransform(app->computeImageTransform({backgroundImage->getWidth(), backgroundImage->getHeight()}) );
    else
      app->setDrawTransform({});
    app->drawImage(*backgroundImage);
  }
}

void GamePhase::setBackground(const Image& image, bool keepAspect) {
  keepBackgroundAspect = keepAspect;
  backgroundImage = std::make_shared<GLTexture2D>(image);
}

void GamePhase::setOverlay(std::shared_ptr<GamePhase> overlay) {
  overlayPhase = overlay;
}

void GamePhase::mouseMove(double xPosition, double yPosition) {
  if (overlayPhase)
    overlayPhase->mouseMove(xPosition, yPosition);
  else
    mouseMoveInternal(xPosition, yPosition);
}

void GamePhase::mouseButton(int button, int state, int mods,
                         double xPosition, double yPosition) {
  if (overlayPhase)
    overlayPhase->mouseButton(button, state, mods,
                              xPosition, yPosition);
  else
    mouseButtonInternal(button, state, mods,
                        xPosition, yPosition);
}

void GamePhase::keyboardChar(unsigned int codepoint) {
  if (overlayPhase)
    overlayPhase->keyboardChar(codepoint);
  else
    keyboardCharInternal(codepoint);
}

void GamePhase::keyboard(int key, int scancode, int action, int mods) {
  if (overlayPhase)
    overlayPhase->keyboard(key, scancode, action, mods);
  else {
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE && gamePhaseID != GamePhaseID::QuitDialog) {
      overlayPhase = std::make_shared<DialogPhase>(app, GamePhaseID::QuitDialog, "Are you sure you want to quit (Y/N) ?");
      overlayPhase->init();
      return;
    }
    keyboardInternal(key, scancode, action, mods);
  }
}

void GamePhase::animate(double animationTime) {
  animateInternal(animationTime);
  if (overlayPhase) {      
    overlayPhase->animate(animationTime);
  }
}

void GamePhase::draw() {
  drawInternal();
  if (overlayPhase) overlayPhase->draw();
}

GamePhaseID GamePhase::getGamePhaseID() const {
  return (overlayPhase) ? overlayPhase->getGamePhaseID() : gamePhaseID;
}
