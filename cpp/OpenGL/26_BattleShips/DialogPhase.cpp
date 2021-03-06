#include "BattleShips.h"

#include "DialogPhase.h"

DialogPhase::DialogPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& prompt) :
GamePhase(app,gamePhaseID),
prompt{prompt},
answer(GLFW_KEY_UNKNOWN)
{
  setBackground(Image{Vec4{0,0,0,0.8f}}, false);
}

void DialogPhase::init() {
  GamePhase::init();
  Image textImage = app->fr.render(prompt);
  texture = GLTexture2D(textImage);
}

void DialogPhase::drawInternal() {
  GamePhase::drawInternal();
  app->setDrawTransform(Mat4::scaling(0.9f) * app->computeImageTransform({texture.getWidth(), texture.getHeight()}) );
  app->drawImage(texture);
}

void DialogPhase::keyboardInternal(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    answer = key;
  }
}
