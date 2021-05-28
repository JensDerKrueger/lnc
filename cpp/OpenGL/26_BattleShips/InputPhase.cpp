#include "BattleShips.h"

#include "InputPhase.h"

InputPhase::InputPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& prompt, const std::string& title) :
GamePhase(app,gamePhaseID),
prompt{prompt},
userInput{},
title{title}
{}

void InputPhase::keyboardInternal(int key, int scancode, int action, int mods) {
  GamePhase::keyboardInternal(key, scancode, action, mods);
  
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

void InputPhase::setPrompt(const std::string& p) {
  prompt = p;
}
  
std::optional<std::string> InputPhase::getInput() const {
  if (complete)
    return userInput;
  else
    return {};
}

void InputPhase::keyboardCharInternal(unsigned int codepoint) {
  GamePhase::keyboardCharInternal(codepoint);
  userInput += char(codepoint);
}

void InputPhase::drawInternal() {
  GamePhase::drawInternal();
  if (!title.empty()) {
    if (backgroundImage) {
      const Vec2 size = app->fe->getSizeFixedWidth(title, app->getAspect(), 0.6f);
      app->setDrawTransform(Mat4::scaling(1.1f*size.x, 1.1f*size.y, 1.0f) * Mat4::translation(Vec3{0.0f, 0.8f, 0.0f}));
      app->drawRect(Vec4(0,0,0,0.7f));
    }
    app->fe->renderFixedWidth(title, app->getAspect(), 0.6f, {0.0f, 0.8f}, Alignment::Center);
  }

  Vec2 promptSize = app->fe->getSizeFixedWidth(prompt, app->getAspect(), 0.4f);
  if (backgroundImage) {
    app->setDrawTransform(Mat4::scaling(promptSize.x+0.01f, 1.1f*promptSize.y, 1.0f) * Mat4::translation(Vec3{0.0f, 0.2f, 0.0f}));
    app->drawRect(Vec4(0,0,0,0.7f));
  }
  app->fe->renderFixedWidth(prompt, app->getAspect(), 0.4f, {0.0f, 0.2f}, Alignment::Center);

  
  if (!userInput.empty()) {
    Vec2 size = app->fe->getSize(userInput, app->getAspect(), promptSize.y);
    if (size.x > 0.95) {
      promptSize = promptSize / (size.x + 0.05f);
      size = size / (size.x + 0.05f);
    }

    if (backgroundImage) {
      app->setDrawTransform(Mat4::scaling(size.x+0.01f, 1.1f*size.y, 1.0f) * Mat4::translation(Vec3{0.0f, -0.2f, 0.0f}));
      app->drawRect(Vec4(0,0,0,0.7f));
    }
    app->fe->render(userInput, app->getAspect(), promptSize.y, {0.0f, -0.2f}, Alignment::Center);
  }


}
