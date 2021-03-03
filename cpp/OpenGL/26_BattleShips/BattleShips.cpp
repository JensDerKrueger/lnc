#include <limits>

#include "BattleShips.h"

#include <Rand.h>

#include "Messages.inc"


#ifndef _WIN32
  #include "helvetica_neue.inc"
  FontRenderer BattleShips::fr{fontImage, fontPos};
#else
  FontRenderer BattleShips::fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
#endif

BattleShips::BattleShips() :
  GLApp(1024, 786, 4, "Online Battleships"),
  adressPhase{this, GamePhaseID::AdressSetup, "Enter server address:"},
  namePhase{this, GamePhaseID::NameSetup, "Enter callsign:"},
  connectingPhase{this, GamePhaseID::Connecting},
  pairingPhase{this, GamePhaseID::Pairing},
  boardSetupPhase{this, GamePhaseID::BoardSetup, boardSize},
  waitingBoardSetupPhase{this, GamePhaseID::WaitingBoardSetup, "Waiting for other Captain"},
  mainPhase{this, GamePhaseID::MainPhase, "Game here"},
  finishedPhase{this, GamePhaseID::Finished}
{}

BattleShips::~BattleShips() {
}

void BattleShips::tryToLoadSettings() {
  std::ifstream settings ("settings.txt");
  std::string line;
  if (settings.is_open()) {
    if (getline(settings,line) && !line.empty() ) {
      serverAddress = line;
      addressComplete = true;
    } 
    if (getline(settings,line) && !line.empty() ) {
      userName = line;
      nameComplete = true;
    }
    settings.close();
  }
}

void BattleShips::init() {
  tryToLoadSettings();

  GL(glEnable(GL_BLEND));
  GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL(glBlendEquation(GL_FUNC_ADD));
  GL(glClearColor(0.0f,0.0f,0.0f,0.0f));

  adressPhase.init();
  namePhase.init();
  connectingPhase.init();
  pairingPhase.init();
  boardSetupPhase.init();
  waitingBoardSetupPhase.init();
  mainPhase.init();
  finishedPhase.init();
}

void BattleShips::mouseMove(double xPosition, double yPosition) {
  if (currentPhase) currentPhase->mouseMove(xPosition, yPosition);
}

void BattleShips::mouseButton(int button, int state, int mods, double xPosition, double yPosition) {
  if (currentPhase) currentPhase->mouseButton(button, state, mods, xPosition, yPosition);
}

void BattleShips::keyboardChar(unsigned int codepoint) {
  if (currentPhase) currentPhase->keyboardChar(codepoint);
}
  
void BattleShips::keyboard(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_ESCAPE) {
      closeWindow();
      return;
    }
  }
  if (currentPhase) currentPhase->keyboard(key, scancode, action, mods);
}

void BattleShips::restartGame() {
  boardSetupPhase.reset();

  myShipPlacement = ShipPlacement{boardSize};
  myBoard = GameGrid{boardSize};
  otherBoard = GameGrid{boardSize};
  password = "";
  encOtherShipPlacement = "";
}


void BattleShips::stateTransition() {
  const GamePhaseID gamePhaseID = currentPhase ? currentPhase->getGamePhaseID() : GamePhaseID::Boot;
      
  if (gamePhaseID > GamePhaseID::Connecting && client && !client->isOK()) {
    currentPhase = &connectingPhase;
  }

  if (gamePhaseID > GamePhaseID::Pairing && !client->getReceivedPairingInfo()) {
    currentPhase = &pairingPhase;
    restartGame();
  }
  
  switch (gamePhaseID) {
    case GamePhaseID::Boot :
      if (!addressComplete) {
        currentPhase = &adressPhase;
      } else {
        if (!nameComplete) {
          currentPhase = &namePhase;
        } else {
          connectingPhase.setText(std::string("Connecting to ") + serverAddress);
          currentPhase = &connectingPhase;
          client = std::make_shared<GameClient>(serverAddress, serverPort, userName, level);
        }
      }
      break;
    case GamePhaseID::AdressSetup :
      if (adressPhase.getInput()) {
        serverAddress = *(adressPhase.getInput());
        currentPhase = &namePhase;
      }
      break;
    case GamePhaseID::NameSetup :
      if (namePhase.getInput()) {
        userName = *(namePhase.getInput());
        connectingPhase.setText(std::string("Connecting to ") + serverAddress);
        currentPhase = &connectingPhase;
        client = std::make_shared<GameClient>(serverAddress, serverPort, userName, level);
      }
      break;
    case GamePhaseID::Connecting :
      if (client->getInitMessageSend()) {
        pairingPhase.setText(pairingMessages[size_t(Rand::rand01() * pairingMessages.size())]);
        currentPhase = &pairingPhase;
      }
      break;
    case GamePhaseID::Pairing :
      if (client->getReceivedPairingInfo()) {
        currentPhase = &boardSetupPhase;
      }
      break;
    case GamePhaseID::BoardSetup :
      if (boardSetupPhase.getPlacement()) {
        myShipPlacement = *(boardSetupPhase.getPlacement());
        password = AESCrypt::genIVString();
        client->sendEncryptedShipPlacement(myShipPlacement.toEncryptedString(password));
        waitingBoardSetupPhase.setText(waitingMessages[size_t(Rand::rand01() * waitingMessages.size())]);
        currentPhase = &waitingBoardSetupPhase;
      }
      break;
    case GamePhaseID::WaitingBoardSetup :
      {
        auto encPlacement = client->getEncryptedShipPlacement();
        if (encPlacement) {
          encOtherShipPlacement = *encPlacement;
          currentPhase = &mainPhase;
        }
      }
    case GamePhaseID::MainPhase :
      break;
    case GamePhaseID::Finished :
      break;
    default:
      std::cout << "oops" << std::endl;
      break;
  }
}

void BattleShips::animate(double animationTime) {
  stateTransition();
  if (currentPhase) currentPhase->animate(animationTime);
}


void BattleShips::draw() {
  if (currentPhase) currentPhase->draw();
}
