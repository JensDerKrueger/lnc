#include <limits>
#include "BattleShips.h"
#include <Rand.h>
#include "Messages.inc"
#include "DialogPhase.h"

#ifndef _WIN32
  #include "helvetica_neue.inc"
  FontRenderer BattleShips::fr{fontImage, fontPos};
#else
  FontRenderer BattleShips::fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
#endif

BattleShips::BattleShips() :
  GLApp(1100, 600, 4, gameTitle, true),
  adressPhase{this, GamePhaseID::AdressSetup, "Enter server address:", gameTitle},
  namePhase{this, GamePhaseID::NameSetup, "Enter callsign:", gameTitle},
  connectingPhase{this, GamePhaseID::Connecting, gameTitle},
  pairingPhase{this, GamePhaseID::Pairing, gameTitle},
  boardSetupPhase{this, GamePhaseID::BoardSetup, boardSize},
  waitingBoardSetupPhase{this, GamePhaseID::WaitingBoardSetup, gameTitle},
  mainPhase{this, GamePhaseID::MainPhase, boardSize},
  finishedPhase{this, GamePhaseID::Finished, boardSize}
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
  fe = fr.generateFontEngine();
  
  tryToLoadSettings();

  adressPhase.init();
  namePhase.init();
  connectingPhase.init();
  pairingPhase.init();
  boardSetupPhase.init();
  waitingBoardSetupPhase.init();
  mainPhase.init();
  finishedPhase.init();

  try {
    Image background = BMP::load("battleship.bmp");
    adressPhase.setBackground(background);
    namePhase.setBackground(background);
    connectingPhase.setBackground(background);
    pairingPhase.setBackground(background);
    boardSetupPhase.setBackground(background);
    waitingBoardSetupPhase.setBackground(background);
    mainPhase.setBackground(background);
    finishedPhase.setBackground(background);
  } catch (const BMP::BMPException& e) {
    std::cerr << e.what() << std::endl;
  }
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
  if (currentPhase) currentPhase->keyboard(key, scancode, action, mods);
}

void BattleShips::restartGame(bool reconnect) {
  myPassword = "";
  encOtherShipPlacement = "";
  otherName  = "";
  otherLevel = 0;
  if (reconnect) {
    currentPhase = nullptr;
  } else {
    client->readyForNewPlayer();
    currentPhase = &pairingPhase;
  }
}

void BattleShips::stateTransition() {
  const GamePhaseID gamePhaseID = currentPhase ? currentPhase->getGamePhaseID() : GamePhaseID::Boot;
      
  if (gamePhaseID > GamePhaseID::Connecting && client && client->isConnecting()) {
    restartGame(true);
  }
  if (gamePhaseID > GamePhaseID::Pairing && gamePhaseID < GamePhaseID::Finished && !client->getReceivedPairingInfo()) {
    restartGame(false);
  }
  
  switch (gamePhaseID) {
    case GamePhaseID::Boot :
      if (!addressComplete) {
        currentPhase = &adressPhase;
      } else {
        if (!nameComplete) {
          currentPhase = &namePhase;
        } else {
          connectingPhase.setSubtitle(std::string("Connecting to ") + serverAddress);
          client = std::make_shared<GameClient>(serverAddress, serverPort, userName, level);
          currentPhase = &connectingPhase;
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
        connectingPhase.setSubtitle(std::string("Connecting to ") + serverAddress);
        client = std::make_shared<GameClient>(serverAddress, serverPort, userName, level);
        currentPhase = &connectingPhase;
      }
      break;
    case GamePhaseID::Connecting :
      if (client->getInitMessageSend()) {
        pairingPhase.setSubtitle(pairingMessages[size_t(Rand::rand01() * pairingMessages.size())]);
        currentPhase = &pairingPhase;
      }
      break;
    case GamePhaseID::Pairing :
      {
        const auto pi = client->getReceivedPairingInfo();
        if (pi) {
          otherName  = limitString(pi->first, 30);
          otherLevel = pi->second;
          boardSetupPhase.prepare();
          currentPhase = &boardSetupPhase;
        }
      }
      break;
    case GamePhaseID::BoardSetup :
      if (boardSetupPhase.getPlacement()) {
        ShipPlacement myShipPlacement = *(boardSetupPhase.getPlacement());
        mainPhase.prepare(myShipPlacement);
        myPassword = AESCrypt::genIVString();
        client->sendEncryptedShipPlacement(myShipPlacement.toEncryptedString(myPassword));
        waitingBoardSetupPhase.setSubtitle(waitingBoardMessages[size_t(Rand::rand01() * waitingBoardMessages.size())]);
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
      break;
    case GamePhaseID::MainPhase :
      if (mainPhase.gameOver()) {
        client->sendShipPlacementPassword(myPassword);
        finishedPhase.prepare(mainPhase.getMyBoard(), mainPhase.getOtherBoard(), encOtherShipPlacement, mainPhase.gameOver());
        currentPhase = &finishedPhase;
      }
      break;
    case GamePhaseID::Finished :
      if (finishedPhase.getTerminate()) {
        restartGame(false);
      }
      break;
    case GamePhaseID::QuitDialog :
      std::shared_ptr<DialogPhase> dialog = std::dynamic_pointer_cast<DialogPhase>(currentPhase->getOverlay());
      if (dialog->getAnswer() == GLFW_KEY_Y || dialog->getAnswer() == GLFW_KEY_Z) {
        closeWindow();
      }
      if (dialog->getAnswer() == GLFW_KEY_N) {
        currentPhase->setOverlay(nullptr);
      }
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

std::string BattleShips::limitString(const std::string& str, size_t maxSize) const {
  if (str.length() <= maxSize) return str;
  return str.substr(0, maxSize-3) + std::string("...");
}
