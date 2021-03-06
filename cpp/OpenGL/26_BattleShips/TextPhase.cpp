#include "BattleShips.h"

#include "TextPhase.h"

TextPhase::TextPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& text) :
GamePhase(app,gamePhaseID),
text{text}
{}

void TextPhase::init() {
  GamePhase::init();
  setText(text);
}

void TextPhase::setText(const std::string& t) {
  text = t;
  baseTextWidth = app->fr.render(text).width;
}

void TextPhase::animateInternal(double animationTime) {
  GamePhase::animateInternal(animationTime);
  animationStep = size_t(animationTime*2) % 4;
}

void TextPhase::drawInternal() {
  GamePhase::drawInternal();
  
  std::string renderText{text};
  if (animationStep > 0) {
    renderText += " ";
    for (size_t i = 0;i<animationStep;++i) {
      renderText += ".";
    }
  }
  Image textImage = app->fr.render(renderText);

  if (backgroundImage) {
    app->setDrawTransform(Mat4::scaling(1.1f*textImage.width / (baseTextWidth * 2.0f)) *
                          app->computeImageTransform({textImage.width, textImage.height}) );
    app->drawRect(Vec4(0,0,0,0.7f));
  }
  
  app->setDrawTransform(Mat4::scaling(textImage.width / (baseTextWidth * 2.0f)) *
                        app->computeImageTransform({textImage.width, textImage.height}) );
  app->drawImage(textImage);  
}
