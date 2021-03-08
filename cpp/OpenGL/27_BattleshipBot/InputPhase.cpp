#include "BattleShips.h"

#include "InputPhase.h"

InputPhase::InputPhase(BattleShips* app, GamePhaseID gamePhaseID, const std::string& prompt) :
GamePhase(app,gamePhaseID),
prompt{prompt}
{}

void InputPhase::run() {
  std::cout << prompt << ": " << std::flush;
  std::cin >> userInput;
}
