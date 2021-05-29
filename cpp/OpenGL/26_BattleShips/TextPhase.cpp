#include "BattleShips.h"

#include "TextPhase.h"

TextPhase::TextPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& title, const std::string& subtitle) :
GamePhase(app,gamePhaseID),
title{title},
subtitle{subtitle}
{}

void TextPhase::init() {
  GamePhase::init();
  setTitle(title);
}

void TextPhase::setTitle(const std::string& text) {
  title = text;
}

void TextPhase::setSubtitle(const std::string& text) {
  subtitle = text;
}

void TextPhase::animateInternal(double animationTime) {
  GamePhase::animateInternal(animationTime);
  animationStep = size_t(animationTime*2) % 4;
}

void TextPhase::drawInternal() {
  GamePhase::drawInternal();
  
  Vec2 shift;
  if (!title.empty()) {
    if (backgroundImage) {
      const Vec2 size = app->fe->getSizeFixedWidth(title, app->getAspect(), 0.8f);
      app->setDrawTransform(Mat4::translation(Vec3{0.0f, 0.2f, 0.0f}) *
                            Mat4::scaling(1.1f*size.x, 1.1f*size.y, 1.0f));
      app->drawRect(Vec4(0,0,0,0.7f));
    }
    app->fe->renderFixedWidth(title, app->getAspect(), 0.8f,  Vec2{0.0f, 0.2f}, Alignment::Center);
    shift = Vec2{0.0f, -0.2f};
  }
  
  const Vec2 basicSize = app->fe->getSizeFixedWidth(subtitle, app->getAspect(), 0.5f);

  std::string renderText{subtitle};
  if (animationStep > 0) {
    renderText += " ";
    for (size_t i = 0;i<animationStep;++i) {
      renderText += ".";
    }
  }

  if (backgroundImage) {
    const Vec2 size = app->fe->getSize(renderText, app->getAspect(), basicSize.y);
    app->setDrawTransform(Mat4::translation(Vec3{shift.x, shift.y, 0.0f}) *
                          Mat4::scaling(1.1f*size.x, 1.1f*size.y, 1.0f));
    app->drawRect(Vec4(0,0,0,0.7f));
  }
  
  app->fe->render(renderText, app->getAspect(), basicSize.y, shift, Alignment::Center);
}
