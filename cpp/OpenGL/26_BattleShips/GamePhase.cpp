#include "BattleShips.h"

#include "GamePhase.h"

void GamePhase::mouseMove(double xPosition, double yPosition) {
  Dimensions s = app->getWindowSize();
  if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;

  normPos = Vec2{float(xPosition/s.width)-0.5f,float(1.0-yPosition/s.height)-0.5f} * 2.0f;
}

void GamePhase::draw() {
  GL(glEnable(GL_BLEND));
  GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL(glBlendEquation(GL_FUNC_ADD));
  
  if (backgroundImage) {
    app->setDrawTransform(app->computeImageTransform({backgroundImage->getWidth(), backgroundImage->getHeight()}) );
    app->drawImage(*backgroundImage);
  } else {
    GL(glClearColor(0.0f,0.0f,0.0f,0.0f));
    GL(glClear(GL_COLOR_BUFFER_BIT));
  }
}

void GamePhase::setBackground(const Image& image) {
  backgroundImage = std::make_shared<GLTexture2D>(image);
}
