#include "BattleShips.h"

#include "BoardSetupPhase.h"

BoardSetupPhase::BoardSetupPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui& b) :
BoardPhase(app, gamePhaseID, b),
myShipPlacement{b}
{}

std::optional<ShipPlacement> BoardSetupPhase::getPlacement() const {
  if (currentPlacement >= ShipPlacement::completePlacement.size())
    return myShipPlacement;
  else
    return {};
}

void BoardSetupPhase::mouseMoveInternal(double xPosition, double yPosition) {
  BoardPhase::mouseMoveInternal(xPosition, yPosition);
    
  const Mat4 invMyBoardTrans = Mat4::inverse(myBoardTrans);
  const Vec2 normMyBoardPos = ((invMyBoardTrans * Vec4(normPos,0.0f,1.0f)).xy() + Vec2{1.0f,1.0f}) / 2.0f;
  
  if (normMyBoardPos.x < 0 || normMyBoardPos.x > 1.0 || normMyBoardPos.y < 0 || normMyBoardPos.y > 1.0) {
    myCellPos = Vec2ui(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max());
  } else {
    myCellPos = Vec2ui{std::min(boardSize.x-1, uint32_t(normMyBoardPos.x * boardSize.x)),
                       std::min(boardSize.y-1, uint32_t(normMyBoardPos.y * boardSize.y))};
  }
}

void BoardSetupPhase::mouseButtonInternal(int button, int state, int mods, double xPosition, double yPosition) {
  BoardPhase::mouseButtonInternal(button, state, mods, xPosition, yPosition);

  if (button == GLFW_MOUSE_BUTTON_RIGHT && state == GLFW_PRESS) {
    toggleOrientation();
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_PRESS) {
    if (myCellPos.x < boardSize.x && myCellPos.y < boardSize.y) {
      if (myShipPlacement.addShip({ShipPlacement::completePlacement[currentPlacement], currentOrientation, myCellPos})) currentPlacement++;
    }
  }

}
void BoardSetupPhase::keyboardInternal(int key, int scancode, int action, int mods) {
  BoardPhase::keyboardInternal(key, scancode, action, mods);
  
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_R :
        toggleOrientation();
        break;
      case GLFW_KEY_U :
        if (currentPlacement > 0) {
          currentPlacement--;
          myShipPlacement.deleteLastShip();
        }
        break;
    }
  }
}

void BoardSetupPhase::drawInternal() {
  BoardPhase::drawInternal();
  if(currentPlacement >= ShipPlacement::completePlacement.size()) return;

  if (backgroundImage) app->drawRect(Vec4(0,0,0,0.7f));

  app->fe->render("Position Your Fleet (Hit R to rotate / hit U to undo placement)", app->getAspect(), 0.05f, {0.0f,0.9f}, Alignment::Center);
  app->fe->render("You are battling " + app->getOtherName(), app->getAspect(), 0.05f, {0.0f,-0.9f}, Alignment::Center);

  myBoardTrans = app->computeImageTransform(boardSize) * Mat4::scaling(0.8f);

  app->setDrawTransform(myBoardTrans);
  app->drawLines(gridLines, LineDrawType::LIST, 3);
  
  const std::vector<Ship>& ships = myShipPlacement.getShips();
  
  bool shipAdded = myShipPlacement.addShip({ShipPlacement::completePlacement[currentPlacement], currentOrientation, myCellPos});

  for (size_t y = 0; y < boardSize.y; ++y) {
    for (size_t x = 0; x < boardSize.x; ++x) {
      
      float tX = (x+0.5f)/boardSize.x*2.0f-1.0f;
      float tY = (y+0.5f)/boardSize.y*2.0f-1.0f;

      app->setDrawTransform(Mat4::scaling(0.9f/boardSize.x,0.9f/boardSize.y,1.0f) *
                            Mat4::translation(tX,tY,0.0f) * myBoardTrans);
      app->drawImage(emptyCell);
    }
  }

  
  for (const Ship& ship : ships) {
    const Vec2ui start = ship.pos;
    const Vec2ui end   = ship.computeEnd();
    for (size_t y = start.y; y <= end.y; ++y) {
      for (size_t x = start.x; x <= end.x; ++x) {
        
        float tX = (x+0.5f)/boardSize.x*2.0f-1.0f;
        float tY = (y+0.5f)/boardSize.y*2.0f-1.0f;

        app->setDrawTransform(Mat4::scaling(1.0f/boardSize.x,1.0f/boardSize.y,1.0f) *
                              Mat4::translation(tX,tY,0.0f) * myBoardTrans);
        app->drawImage(shipCell);
      }
    }
  }
  
  if (shipAdded)
    myShipPlacement.deleteLastShip();
}

void BoardSetupPhase::toggleOrientation() {
  currentOrientation = Orientation(1-uint32_t(currentOrientation));
}

void BoardSetupPhase::prepare() {
  myCellPos = Vec2ui{std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()};
  currentOrientation = Orientation::Vertical;
  currentPlacement = 0;
  myShipPlacement = ShipPlacement{boardSize};
}
