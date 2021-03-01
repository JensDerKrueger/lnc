#include "BattleShips.h"

#include <Rand.h>

#ifndef _WIN32
  #include "helvetica_neue.inc"
  FontRenderer BattleShips::fr{fontImage, fontPos};
#else
  FontRenderer BattleShips::fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
#endif

BattleShips::BattleShips() : GLApp(1024, 786, 4, "Online Battleships") {}

BattleShips::~BattleShips() {
}

void BattleShips::tryToLoadSettings() {
  std::ifstream settings ("settings.txt");
  std::string line;
  if (settings.is_open()) {
    if (getline(settings,line) ) {
      serverAddress = line;
      addressComplete = true;
    } 
    if (getline(settings,line) ) {
      userName = line;
      nameComplete = true;
    }
    settings.close();
  }
}

void BattleShips::init() {
  tryToLoadSettings();

  glEnv.setCursorMode(CursorMode::HIDDEN);
  GL(glEnable(GL_BLEND));
  GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL(glBlendEquation(GL_FUNC_ADD));
  GL(glClearColor(0.0f,0.0f,0.0f,0.0f));
    
  gridLines = gridToLines();
}

void BattleShips::updateMousePos() {
  Dimensions s = glEnv.getWindowSize();
  normPos = Vec2{float(xPositionMouse/s.width)-0.5f,float(1.0-yPositionMouse/s.height)-0.5f} * 2.0f;
}

void BattleShips::mouseMove(double xPosition, double yPosition) {
  if (!client) return;
  
  Dimensions s = glEnv.getWindowSize();
  if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
  
  xPositionMouse = xPosition;
  yPositionMouse = yPosition;
  updateMousePos();
    
}

void BattleShips::mouseButton(int button, int state, int mods, double xPosition, double yPosition) {
  if (!client) return;
  
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    rightMouseDown = (state == GLFW_PRESS);
  }

}


void BattleShips::keyboardChar(unsigned int codepoint) {
  if (!client) {
    if (!addressComplete) {
      serverAddress += char(codepoint);
      responseImage = fr.render(serverAddress);
    } else {
      userName += char(codepoint);
      responseImage = fr.render(userName);
    }
  }
}
  
void BattleShips::keyboard(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    
    if (key == GLFW_KEY_ESCAPE) {
      closeWindow();
      return;
    }

    if (!client) {
      std::string& str = addressComplete ? userName : serverAddress;
      bool& complete = addressComplete ? nameComplete : addressComplete;
      switch (key) {
        case GLFW_KEY_BACKSPACE :
          if (str.size() > 0) str.erase(str.size() - 1);
          responseImage = fr.render(str);
          break;
        case GLFW_KEY_ENTER :
          if (str.size() > 0) complete = true;
          break;
      }
      return;
    }
  }
}

void BattleShips::animate(double animationTime) {
  currentImage = size_t(animationTime*2);
  
  if (gameState > GameState::Connecting && client && !client->isOK())
    gameState = GameState::Connecting;
  
  switch (gameState) {
    case GameState::Startup :
      if (nameComplete && addressComplete) {
        client = std::make_shared<GameClient>(serverAddress, serverPort, userName, level);
        gameState = GameState::Connecting;
      }
      break;
    case GameState::Connecting :
      if (client->getInitMessageSend()) {
        gameState = GameState::Pairing;
        pairingMessage = size_t(Rand::rand01() * pairingMessages.size());
      }
      break;
    case GameState::Pairing :
      if (client->getReceivedPairingInfo()) {
        gameState = GameState::BoardSetup;
      }
      break;
    case GameState::BoardSetup :
      if (shipsPlaced) {
        password = AESCrypt::genIVString();
        client->sendEncryptedShipPlacement(myShipPlacement.toEncryptedString(password));
        gameState = GameState::WaitingBoardSetup;
      }
      break;
    case GameState::WaitingBoardSetup :
      {
        auto encPlacement = client->getEncryptedShipPlacement();
        if (encPlacement) {
          encOtherShipPlacement = *encPlacement;
          gameState = GameState::Firing;
        }
      }
      break;
    default:
      std::cout << "oops" << std::endl;
      break;
  }
}

void BattleShips::drawStartup() {
  if (serverAddress.empty()) {
    responseImage = fr.render("Type in server address:");
  } else if (addressComplete && userName.empty()) {
    responseImage = fr.render("Type in your name:");
  }
  setDrawTransform(Mat4::scaling(1.0f/3.0f) * computeImageTransform({responseImage.width, responseImage.height}) );
  drawImage(responseImage);
}

void BattleShips::drawConnecting() {
  Image connectingImage = fr.render("Connecting to " + serverAddress);
  const size_t baseTextWidth = connectingImage.width;
  
  switch (currentImage % 4) {
    default:
      break;
    case 1 :
      connectingImage = fr.render("Connecting to " + serverAddress + " .");
      break;
    case 2 :
      connectingImage = fr.render("Connecting to " + serverAddress + " ..");
      break;
    case 3 :
      connectingImage = fr.render("Connecting to " + serverAddress + " ...");
      break;
  }

  setDrawTransform(Mat4::scaling(connectingImage.width / (baseTextWidth * 2.0f)) * computeImageTransform({connectingImage.width, connectingImage.height}) );
  drawImage(connectingImage);
}

void BattleShips::drawPairing() {
  Image connectingImage = fr.render(pairingMessages[pairingMessage]);
  const size_t baseTextWidth = connectingImage.width;
  
  switch (currentImage % 4) {
    default:
      break;
    case 1 :
      connectingImage = fr.render(pairingMessages[pairingMessage] + " .");
      break;
    case 2 :
      connectingImage = fr.render(pairingMessages[pairingMessage] + " ..");
      break;
    case 3 :
      connectingImage = fr.render(pairingMessages[pairingMessage] + " ...");
      break;
  }

  setDrawTransform(Mat4::scaling(connectingImage.width / (baseTextWidth * 2.0f)) * computeImageTransform({connectingImage.width, connectingImage.height}) );
  drawImage(connectingImage);
}


std::vector<float> BattleShips::gridToLines() const {
  std::vector<float> lines;
  
  const uint32_t w = myBoard.getSize().x();
  const uint32_t h = myBoard.getSize().y();
  
  for (uint32_t y = 0;y<h+1;++y) {
    lines.push_back(-1);
    lines.push_back(1.0f-2.0f*float(y)/h);
    lines.push_back(0);
    
    lines.push_back(1);
    lines.push_back(1);
    lines.push_back(1);
    lines.push_back(1);
    
    lines.push_back(1);
    lines.push_back(1.0f-2.0f*float(y)/h);
    lines.push_back(0);

    lines.push_back(1);
    lines.push_back(1);
    lines.push_back(1);
    lines.push_back(1);
  }

  for (size_t x = 0;x<w+1;++x) {
    lines.push_back(1.0f-2.0f*float(x)/w);
    lines.push_back(-1);
    lines.push_back(0);
    
    lines.push_back(1);
    lines.push_back(1);
    lines.push_back(1);
    lines.push_back(1);
    
    lines.push_back(1.0f-2.0f*float(x)/w);
    lines.push_back(1);
    lines.push_back(0);

    lines.push_back(1);
    lines.push_back(1);
    lines.push_back(1);
    lines.push_back(1);
  }
  
  return lines;
}

void BattleShips::drawBoards() {
  setDrawTransform(Mat4::scaling(0.8f,0.8f,0.8f) * Mat4::translation(0.0f,0.0f,0.0f));
  drawLines(gridLines, LineDrawType::LIST, 3);

  if (gameState != GameState::BoardSetup) {
    setDrawTransform(Mat4::scaling(0.8f,0.8f,0.8f) * Mat4::translation(0.0f,0.0f,0.0f));
    drawLines(gridLines, LineDrawType::LIST, 3);
  }
}


void BattleShips::draw() {
  GL(glClearColor(0.0f,0.0f,0.0f,0.0f));
  GL(glClear(GL_COLOR_BUFFER_BIT));
  
  switch (gameState) {
    case GameState::Startup :
      drawStartup();
      break;
    case GameState::Connecting :
      drawConnecting();
      break;
    case GameState::Pairing :
      drawPairing();
      break;
    case GameState::BoardSetup :
    case GameState::Firing :
    case GameState::WaitingFiring :
      drawBoards();
      break;
    default:
      std::cout << "oops draw" << std::endl;
      break;
  }
  
}
