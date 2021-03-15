#include <limits>
#include "BattleShips.h"
#include <Rand.h>

BattleShips::BattleShips() :
  adressPhase{this, GamePhaseID::AdressSetup, "Enter server address:"},
  connectingPhase{this, GamePhaseID::Connecting},
  pairingPhase{this, GamePhaseID::Pairing},
  boardSetupPhase{this, GamePhaseID::BoardSetup, boardSize},
  waitingBoardSetupPhase{this, GamePhaseID::WaitingBoardSetup, "Waiting for other Captain"},
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
    settings.close();
  }

  std::ifstream otherCallFile ("callsigns.txt");
  if (otherCallFile.is_open()) {
    while (getline(otherCallFile,line) && !line.empty() ) {
      otherCallsigns.push_back(line);
    }
    otherCallFile.close();
  }

  std::ifstream nastyPosFile ("nastyPos.txt");
  try {
    if (nastyPosFile.is_open()) {
      while (getline(nastyPosFile,line) && !line.empty() ) {
        Tokenizer tokenizer{line+",", ','};
        const uint32_t x = tokenizer.nextUint32();
        const uint32_t y = tokenizer.nextUint32();
        nastyPos.push_back({x,y});
      }
      nastyPosFile.close();
    }
  } catch (const MessageException& ) {
  }
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

void BattleShips::run() {
  tryToLoadSettings();
  bool terminate{false};
  
  do {
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
          connectingPhase.setText(std::string("Connecting to ") + serverAddress);
          client = std::make_shared<GameClient>(serverAddress, serverPort, generateName(), level);
          currentPhase = &connectingPhase;
        }
        break;
      case GamePhaseID::AdressSetup :
        serverAddress = adressPhase.getInput();
        connectingPhase.setText(std::string("Connecting to ") + serverAddress);
        client = std::make_shared<GameClient>(serverAddress, serverPort, generateName(), level);
        currentPhase = &connectingPhase;
        break;
      case GamePhaseID::Connecting :
        if (client->getInitMessageSend()) {
          pairingPhase.setText("Wating for partner");
          currentPhase = &pairingPhase;
        }
        break;
      case GamePhaseID::Pairing :
        {
          const auto pi = client->getReceivedPairingInfo();
          if (pi) {
            otherName  = limitString(pi->first, 30);
            otherLevel = pi->second;
            rememberName(pi->first);
            boardSetupPhase.prepare();
            currentPhase = &boardSetupPhase;
          }
        }
        break;
      case GamePhaseID::BoardSetup :
        {
          ShipPlacement myShipPlacement = boardSetupPhase.getPlacement();
          mainPhase.prepare(myShipPlacement);
          myPassword = AESCrypt::genIVString();
          client->sendEncryptedShipPlacement(myShipPlacement.toEncryptedString(myPassword));
          waitingBoardSetupPhase.setText("Waiting for partner's placement");
          currentPhase = &waitingBoardSetupPhase;
        }
        break;
      case GamePhaseID::WaitingBoardSetup :
        {
          auto encPlacement = client->getEncryptedShipPlacement();
          if (encPlacement) {
            encOtherShipPlacement = *encPlacement;
            currentPhase = &mainPhase;
            std::cout << "Game starting" << std::endl;
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
          std::cout << "I ";
          if (finishedPhase.getCheater()) {
            std::cout << "won (other cheated)";
            cheaters++;
          } else {
            if (mainPhase.gameOver() == 1) {
              std::cout << "won";
              winns++;
            } else {
              std::cout << "lost";
              losses++;
            }
          }
          std::cout << " against " << otherName << "\nWinns:" << winns << " Losses:" << losses << " Cheater:" << cheaters << std::endl;
          restartGame(true);
        }
        break;
    }
    if (currentPhase) currentPhase->run();
    std::this_thread::sleep_for(std::chrono::milliseconds(10) );
  } while (!terminate);
}

std::string BattleShips::limitString(const std::string& str, size_t maxSize) const {
  if (str.length() <= maxSize) return str;
  return str.substr(0, maxSize-3) + std::string("...");
}

std::string BattleShips::generateName() const {
  const std::vector<std::string> callsignsAddons{
    "Hunter",
    "Killer",
    "Destroyer"
  };
  const std::vector<std::string> nouns {
    "Hunter",
    "Killer",
    "Captian",
    "Admiral",
    "Wolf",
    "Wolfpack",
    "Lion",
    "Tiger",
    "Destroyer",
    "Devil",
    "Saint"
  };
  const std::vector<std::string> adjectives {
    "Great",
    "Little",
    "Nasty",
    "Tiny",
    "Invisible",
    "Nightmarish",
    "Powerful",
    "Strong",
    "Superior",
    "Black",
    "Red",
    "White",
    "Pink",
  };

  std::string firstPart;
  std::string secondPart;
  
  if (otherCallsigns.size() > 0 && Rand::rand01() > 0.5) {
    firstPart = otherCallsigns[Rand::rand<size_t>(0, otherCallsigns.size())];
    secondPart = callsignsAddons[Rand::rand<size_t>(0, callsignsAddons.size())];

    if (Rand::rand01() > 0.2) {
      return firstPart + " " + secondPart;
    } else {
      return firstPart;
    }
  } else {
    firstPart = adjectives[Rand::rand<size_t>(0, adjectives.size())];
    secondPart = nouns[Rand::rand<size_t>(0, nouns.size())];

    if (Rand::rand01() > 0.2) {
      return firstPart + " " + secondPart;
    } else {
      return secondPart;
    }
  }
}


void BattleShips::rememberNastyPos(const GameGrid& fullOtherBoard) {
  const size_t MAX_NASTY_POS = 50;
  
  std::vector<Vec2ui> newNastyPos;
  for (uint32_t y = 0;y<fullOtherBoard.getSize().y();++y) {
    for (uint32_t x = 0;x<fullOtherBoard.getSize().x();++x) {
        if (fullOtherBoard.getCell(x,y) == Cell::Ship)
          newNastyPos.push_back({x,y});
    }
  }
  
  for (size_t i = 0;i<newNastyPos.size();++i) {
    for (size_t j = 0;j<nastyPos.size();++j) {
      if (newNastyPos[i] == nastyPos[j]) {
        std::swap(newNastyPos[i], newNastyPos.back());
        newNastyPos.pop_back();
        i--;
        break;
      }
    }
  }

  nastyPos.insert(nastyPos.end(), newNastyPos.begin(), newNastyPos.end());
  if (nastyPos.size() > MAX_NASTY_POS) {
    Rand::shuffle(newNastyPos);
    nastyPos.resize(MAX_NASTY_POS);
  }
    
  std::ofstream nastyPosFile ("nastyPos.txt");
  if (nastyPosFile.is_open()) {
    for (const Vec2ui& p : nastyPos) {
      nastyPosFile << p.x() << "," << p.y() << std::endl;
    }
    nastyPosFile.close();
  }
}


void BattleShips::rememberName(const std::string& str) {
  bool notFound{true};
  
  for (const std::string& callsign : otherCallsigns) {
    if (str == callsign) {
      notFound = false;
      break;
    }
  }
  
  if (notFound) {
    otherCallsigns.push_back(str);
    std::ofstream otherCallFile ("callsigns.txt");
    if (otherCallFile.is_open()) {
      for (const std::string& callsign : otherCallsigns) {
        otherCallFile << callsign << std::endl;
      }
    }
    otherCallFile.close();
  }
}

Vec2ui BattleShips::getRandomNastyPos() const {
  if (nastyPos.empty()) return {0,0};
  return nastyPos[Rand::rand<size_t>(0,nastyPos.size())];
}
