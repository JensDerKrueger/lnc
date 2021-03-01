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

  GL(glEnable(GL_BLEND));
  GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL(glBlendEquation(GL_FUNC_ADD));
  GL(glClearColor(0.0f,0.0f,0.0f,0.0f));
    
  Image cellImage{10,10};
  for (uint32_t y = 0;y<cellImage.width;++y) {
    for (uint32_t x = 0;x<cellImage.height;++x) {
      cellImage.setNormalizedValue(x, y, 0, 0.0f);
      cellImage.setNormalizedValue(x, y, 1, 0.0f);
      cellImage.setNormalizedValue(x, y, 2, 1.0f);
      cellImage.setNormalizedValue(x, y, 3, 1.0f);
    }
  }
  emptyCell = GLTexture2D{cellImage};
  for (uint32_t y = 0;y<cellImage.width;++y) {
    for (uint32_t x = 0;x<cellImage.height;++x) {
      cellImage.setNormalizedValue(x, y, 0, 0.3f);
      cellImage.setNormalizedValue(x, y, 1, 0.3f);
      cellImage.setNormalizedValue(x, y, 2, 0.3f);
      cellImage.setNormalizedValue(x, y, 3, 1.0f);
    }
  }
  unknownCell = GLTexture2D{cellImage};
  
  for (uint32_t y = 0;y<cellImage.width;++y) {
    for (uint32_t x = 0;x<cellImage.height;++x) {
      cellImage.setNormalizedValue(x, y, 0, 1.0f);
      cellImage.setNormalizedValue(x, y, 1, 0.0f);
      cellImage.setNormalizedValue(x, y, 2, 0.0f);
      cellImage.setNormalizedValue(x, y, 3, 1.0f);
    }
  }
  shipCell = GLTexture2D{cellImage};
  
  gridLines = gridToLines();
}

void BattleShips::updateMousePos() {
  Dimensions s = glEnv.getWindowSize();
  normPos = Vec2{float(xPositionMouse/s.width)-0.5f,float(1.0-yPositionMouse/s.height)-0.5f} * 2.0f;
  
  const Mat4 invMyBoardTrans = Mat4::inverse(myBoardTrans);
  const Vec2 normMyBoardPos = ((invMyBoardTrans * Vec4(normPos,0,1)).xy() + Vec2{1.0f,1.0f}) / 2.0f;
  myCellPos = Vec2ui{std::min(boardSize.x()-1, uint32_t(normMyBoardPos.x() * boardSize.x())),
                     std::min(boardSize.y()-1, uint32_t(normMyBoardPos.y() * boardSize.y()))};

  const Mat4 invOtherBoardTrans = Mat4::inverse(otherBoardTrans);
  const Vec2 normOtherBoardPos = ((invOtherBoardTrans * Vec4(normPos,0,1)).xy() + Vec2{1.0f,1.0f}) / 2.0f;
  otherCellPos = Vec2ui{std::min(boardSize.x()-1, uint32_t(normOtherBoardPos.x() * boardSize.x())),
                        std::min(boardSize.y()-1, uint32_t(normOtherBoardPos.y() * boardSize.y()))};

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
  
  if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    rightMouseDown = (state == GLFW_PRESS);
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    leftMouseDown = (state == GLFW_PRESS);
  }

  if (rightMouseDown) {
    toggleOrientation();
  }

  if (leftMouseDown) {
    switch (gameState) {
      case GameState::BoardSetup :
        myShipPlacement.addShip({placementOrder[currentPlacement], currentOrientation, myCellPos});
        currentPlacement++;
        break;
      default:
        break;
    }
  }
}


void BattleShips::toggleOrientation() {
  currentOrientation = Orientation(1-uint32_t(currentOrientation));
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

    switch (gameState) {
      case GameState::Startup : {
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
        break;
      case GameState::BoardSetup : {
          switch (key) {
            case GLFW_KEY_R :
              toggleOrientation();
              break;
            case GLFW_KEY_U :
              if (currentPlacement > 0) {
                currentPlacement--;
                myShipPlacement.deleteShipAt(currentPlacement);
              }
              break;
          }
        }
        break;
      default:
        break;
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
      if (currentPlacement == placementOrder.size()) {
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

void BattleShips::drawBoardSetup() {
  if(currentPlacement >= placementOrder.size()) return;
  
  myBoardTrans = Mat4::scaling(0.8f,0.8f,1.0f) * Mat4::translation(0.0f,0.0f,0.0f);

  setDrawTransform(myBoardTrans);
  drawLines(gridLines, LineDrawType::LIST, 3);
  
  const std::vector<Ship>& ships = myShipPlacement.getShips();
  
  bool shipAdded = myShipPlacement.addShip({placementOrder[currentPlacement], currentOrientation, myCellPos});
   
  
  for (const Ship& ship : ships) {
    const Vec2ui start = ship.pos;
    const Vec2ui end   = ship.computeEnd();
    for (size_t y = start.y(); y <= end.y(); ++y) {
      for (size_t x = start.x(); x <= end.x(); ++x) {
        
        float tX = (x+0.5f)/boardSize.x()*2.0f-1.0f;
        float tY = (y+0.5f)/boardSize.y()*2.0f-1.0f;

        setDrawTransform( Mat4::scaling(1.0f/boardSize.x(),1.0f/boardSize.y(),1.0f) * Mat4::translation(tX,tY,0.0f) * myBoardTrans);
        drawImage(shipCell);
      }
    }
  }
  
  if (shipAdded)
    myShipPlacement.deleteShipAt(ships.size()-1);
  
}


void BattleShips::drawBoards() {
  myBoardTrans = Mat4::scaling(0.8f,0.8f,0.8f) * Mat4::translation(0.0f,0.0f,0.0f);
  otherBoardTrans = Mat4::scaling(0.8f,0.8f,0.8f) * Mat4::translation(0.0f,0.0f,0.0f);

  setDrawTransform(myBoardTrans);
  drawLines(gridLines, LineDrawType::LIST, 3);

  setDrawTransform(otherBoardTrans);
  drawLines(gridLines, LineDrawType::LIST, 3);
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
      drawBoardSetup();
      break;
    case GameState::Firing :
    case GameState::WaitingFiring :
      drawBoards();
      break;
    default:
      std::cout << "oops draw" << std::endl;
      break;
  }
  
}
