#include "BattleShips.h"

#include "TextPhase.h"

TextPhase::TextPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& text) :
GamePhase(app,gamePhaseID),
text{text}
{}

void TextPhase::run() {
  if (!shown) {
    std::cout << text << std::endl;
    shown = true;
  }
}

void TextPhase::setText(const std::string& t) {
  text = t;
  shown = false;
}
