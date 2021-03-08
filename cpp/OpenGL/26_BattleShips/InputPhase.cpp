#include "BattleShips.h"

#include "InputPhase.h"

InputPhase::InputPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& prompt, const std::string& title) :
GamePhase(app,gamePhaseID),
prompt{prompt},
userInput{},
title{title}
{}

void InputPhase::init() {
  GamePhase::init();
  titleTex = GLTexture2D(app->fr.render(title));
}

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
    const Mat4 titleTrans = app->computeImageTransformFixedHeight({titleTex.getWidth(), titleTex.getHeight()}, 0.1f, Vec3{0.f,0.8f,0.0f});
    app->setDrawTransform(titleTrans);
    app->drawImage(titleTex);
  }  
  Image textImage = app->fr.render((userInput.size() > 0) ? userInput : prompt);
  if (backgroundImage) {
    app->setDrawTransform(Mat4::scaling(1.1f) * app->computeImageTransform({textImage.width, textImage.height}) );
    app->drawRect(Vec4(0,0,0,0.7f));
  }
  app->setDrawTransform(Mat4::scaling(0.5f) * app->computeImageTransform({textImage.width, textImage.height}) );
  app->drawImage(textImage);
}
