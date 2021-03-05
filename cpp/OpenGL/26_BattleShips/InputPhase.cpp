#include "BattleShips.h"

#include "InputPhase.h"

InputPhase::InputPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& prompt) :
GamePhase(app,gamePhaseID),
prompt{prompt}
{}

void InputPhase::keyboard(int key, int scancode, int action, int mods) {
  GamePhase::keyboard(key, scancode, action, mods);
  
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_BACKSPACE :
        if (userInput.size() > 0) userInput.erase(userInput.size() - 1);
        break;
      case GLFW_KEY_ENTER :
        if (userInput.size() > 0) complete = true;
        break;
    }
  }
}

void InputPhase::keyboardChar(unsigned int codepoint) {
  GamePhase::keyboardChar(codepoint);
  
  userInput += char(codepoint);
}

void InputPhase::draw() {
  GamePhase::draw();
  Image textImage = app->fr.render((userInput.size() > 0) ? userInput : prompt);

  if (backgroundImage) {
    app->setDrawTransform(app->computeImageTransform({backgroundImage->getWidth(), backgroundImage->getHeight()}) );
    app->drawImage(*backgroundImage);
    app->setDrawTransform(Mat4::scaling(1.1f) *
                          app->computeImageTransform({textImage.width, textImage.height}) );
    app->drawRect(Vec4(0,0,0,0.7f));
  }

  app->setDrawTransform(Mat4::scaling(0.5f) *
                        app->computeImageTransform({textImage.width, textImage.height}) );
  app->drawImage(textImage);
}
