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
  titleTex = GLTexture2D(app->fr.render(title));
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
  
  Vec3 shift;
  if (!title.empty()) {
    Mat4 titleTrans = app->computeImageTransformFixedWidth({titleTex.getWidth(), titleTex.getHeight()}, 0.8f, Vec3{0.0f, 0.2f, 0.0f});
    if (backgroundImage) {
      app->setDrawTransform(Mat4::scaling(1.1f, 1.1f, 1.0f) * titleTrans);
      app->drawRect(Vec4(0,0,0,0.7f));
    }
    app->setDrawTransform(titleTrans);
    app->drawImage(titleTex);
    shift = Vec3{0.0f, -0.2f, 0.0f};
  }
  
  std::string renderText{subtitle};
  if (animationStep > 0) {
    renderText += " ";
    for (size_t i = 0;i<animationStep;++i) {
      renderText += ".";
    }
  }

  Image subtitleImage = app->fr.render(renderText);
  Mat4 subtitleTrans = app->computeImageTransformFixedHeight({subtitleImage.width, subtitleImage.height}, 0.1f, shift);
  
  if (backgroundImage) {
    app->setDrawTransform(Mat4::scaling(1.1f, 1.1f, 1.0f) * subtitleTrans);
    app->drawRect(Vec4(0,0,0,0.7f));
  }
  
  app->setDrawTransform(subtitleTrans);
  app->drawImage(subtitleImage);
}
