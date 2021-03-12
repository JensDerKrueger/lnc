#include "BattleShips.h"

#include "DialogPhase.h"

DialogPhase::DialogPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& prompt) :
GamePhase(app,gamePhaseID),
prompt{prompt},
answer(GLFW_KEY_UNKNOWN)
{
  setBackground(Image{Vec4{0,0,0,0.8f}}, false);
}

void DialogPhase::drawInternal() {
  GamePhase::drawInternal();
  app->fe->renderFixedWidth(prompt, app->getAspect(), 0.9f, Vec2{}, Alignment::Center);
}

void DialogPhase::keyboardInternal(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    answer = key;
  }
}
