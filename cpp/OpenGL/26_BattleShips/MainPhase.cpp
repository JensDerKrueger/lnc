#include "BattleShips.h"

#include "MainPhase.h"
#include "Messages.inc"

MainPhase::MainPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui& b) :
BoardPhase(app, gamePhaseID, b)
{
}

void MainPhase::prepare(const ShipPlacement& myShipPlacement) {
  myBoard = GameGrid{boardSize};
  myBoard.setShips(myShipPlacement);

  otherBoard = GameGrid{boardSize};
  otherBoard.clearUnknown();

  homeTitle  = homeTitles[size_t(Rand::rand01() * homeTitles.size())];
  guestTitle = guestTitles[size_t(Rand::rand01() * guestTitles.size())];

  waitingForOther = false;
  waitingMessageIndex = 0;
    
  myRound = 0;
  otherRound = 0;
  shotsFired.clear();
  shotsResponded.clear();
  shotsReceived.clear();
  shotResults.clear();
}

void MainPhase::mouseMoveInternal(double xPosition, double yPosition) {
  BoardPhase::mouseMoveInternal(xPosition, yPosition);
    
  const Mat4 invOtherBoardTrans = Mat4::inverse(otherBoardTrans);
  const Vec2 normOtherBoardPos = ((invOtherBoardTrans * Vec4(normPos,0,1)).xy() + Vec2{1.0f,1.0f}) / 2.0f;
  
  if (normOtherBoardPos.x() < 0 || normOtherBoardPos.x() > 1.0 || normOtherBoardPos.y() < 0 || normOtherBoardPos.y() > 1.0) {
    otherCellPos = Vec2ui(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max());
  } else {
    otherCellPos = Vec2ui{std::min(boardSize.x()-1, uint32_t(normOtherBoardPos.x() * boardSize.x())),
                          std::min(boardSize.y()-1, uint32_t(normOtherBoardPos.y() * boardSize.y()))};
  }
  app->getClient()->aimAt(otherCellPos);
}

void MainPhase::mouseButtonInternal(int button, int state, int mods, double xPosition, double yPosition) {
  BoardPhase::mouseButtonInternal(button, state, mods, xPosition, yPosition);

  if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_PRESS) {
    if (shotResults.size() == shotsFired.size() &&
        otherCellPos.x() < boardSize.x() && otherCellPos.y() < boardSize.y() && ! waitingForOther &&
        Cell::Unknown == otherBoard.getCell(otherCellPos.x(), otherCellPos.y())) {
      shotsFired.push_back(otherCellPos);
      app->getClient()->shootAt(otherCellPos);
    }
  }
}

void MainPhase::drawBoard(const GameGrid& board, Mat4 boardTrans, Vec2ui aimCoords) {
  for (uint32_t y = 0; y < boardSize.y(); ++y) {
    for (uint32_t x = 0; x < boardSize.x(); ++x) {
      
      float tX = (x+0.5f)/boardSize.x()*2.0f-1.0f;
      float tY = (y+0.5f)/boardSize.y()*2.0f-1.0f;
      app->setDrawTransform(Mat4::scaling(0.9f/boardSize.x(),0.9f/boardSize.y(),1.0f) * Mat4::translation(tX,tY,0.0f) * boardTrans);
      
      switch (board.getCell(x,y)) {
        case Cell::Unknown :
          app->drawImage(unknownCell);
          break;
        case Cell::Empty :
          app->drawImage(emptyCell);
          break;
        case Cell::Ship :
          app->drawImage(shipCell);
          break;
        case Cell::ShipShot :
          app->drawImage(shipCell);
          app->drawImage(shotCell);
          break;
        case Cell::EmptyShot :
          app->drawImage(emptyCell);
          app->drawImage(shotCell);
          break;
      }
      
      if (x == aimCoords.x() && y == aimCoords.y())
        app->drawImage(aimCell);
    }
  }
}

void MainPhase::drawInternal() {
  BoardPhase::drawInternal();
  
  if (backgroundImage) {
    app->drawRect(Vec4(0,0,0,0.7f));
  }
  
  Image name;
  if (sunkShipWithLastShot)
    name = app->fr.render("You sunk one of the ships from " + app->getOtherName());
  else
    name = app->fr.render("You are battling " + app->getOtherName());
  
  Mat4 imageTrans = app->computeImageTransform({name.width, name.height});
  Vec2 realImageSize = (imageTrans * Vec4(float(name.width), float(name.height), 0, 1)).xy();
  app->setDrawTransform(imageTrans * Mat4::scaling(5.0f/realImageSize.y()) * Mat4::translation(0.0f,-0.85f,0.0f));
  app->drawImage(name);
  
  Image name1 = app->fr.render(homeTitle);
  Image name2 = app->fr.render(guestTitle);
  
  uint32_t maxWidth = std::max(name1.width, name2.width);

  app->setDrawTransform(app->computeImageTransform({name1.width, name1.height}) * Mat4::scaling((0.3f*name1.width)/maxWidth) * Mat4::translation(-0.5f,0.8f,0.0f));
  app->drawImage(name1);

  app->setDrawTransform(app->computeImageTransform({name2.width, name2.height}) * Mat4::scaling((0.3f*name2.width)/maxWidth) * Mat4::translation( 0.5f,0.8f,0.0f));
  app->drawImage(name2);

  Mat4 myBoardTrans = app->computeImageTransform(boardSize) * Mat4::scaling(0.6f) * Mat4::translation(-0.5f,0.0f,0.0f);
  app->setDrawTransform(myBoardTrans);
  app->drawLines(gridLines, LineDrawType::LIST, 3);
  
  drawBoard(myBoard, myBoardTrans, app->getClient()->getAim());

  otherBoardTrans = app->computeImageTransform(boardSize) * Mat4::scaling(0.6f) * Mat4::translation(0.5f,0.0f,0.0f);
  app->setDrawTransform(otherBoardTrans);
  app->drawLines(gridLines, LineDrawType::LIST, 3);

  drawBoard(otherBoard, otherBoardTrans, otherCellPos);
  
  if (waitingForOther) {
    Image prompt = app->fr.render(waitingShotMessages[waitingMessageIndex]);
    app->setDrawTransform(app->computeImageTransform({prompt.width, prompt.height}) * Mat4::scaling(0.9f));
    app->drawRect(Vec4(0,0,0,0.7f));
    app->setDrawTransform(app->computeImageTransform({prompt.width, prompt.height}) * Mat4::scaling(0.8f));
    app->drawImage(prompt);
  }
}

void MainPhase::animateInternal(double animationTime) {
  BoardPhase::animateInternal(animationTime);
  
  const std::vector<Vec2ui> newShotsReceived = app->getClient()->getShotsReceived();
  const std::vector<ShotResult> newShotResults = app->getClient()->getShotResults();
  
  if (newShotsReceived.size() > shotsReceived.size() && otherRound <= myRound) {
    Vec2ui newShot = newShotsReceived[shotsReceived.size()];
    
    
    const bool hit = Cell::Ship == myBoard.getCell(newShot.x(), newShot.y()) ||
                     Cell::ShipShot == myBoard.getCell(newShot.x(), newShot.y());
    
    ShotResult result = ShotResult::MISS;
    if (hit) {
      if (myBoard.shipSunk(newShot))
        result = ShotResult::SUNK;
      else
        result = ShotResult::HIT;
    }
    
    if (result == ShotResult::MISS) {
      otherRound++;
    }
    app->getClient()->sendShotResult(result);
    myBoard.addShot({newShot.x(), newShot.y()});
    shotsReceived.push_back(newShot);
  }
  
  if (newShotResults.size() > shotResults.size()) {
    const ShotResult newResult = newShotResults[shotResults.size()];
    const Vec2ui pos = shotsFired[shotResults.size()];
    
    if (newResult == ShotResult::MISS) {
      myRound++;
      otherBoard.addMiss({pos.x(), pos.y()});
    } else {
      otherBoard.addHit({pos.x(), pos.y()}, newResult == ShotResult::SUNK);
    }
    
    sunkShipWithLastShot = (newResult == ShotResult::SUNK);
    
    shotResults.push_back(newResult);
  }

  if (!waitingForOther && myRound > otherRound) {
    waitingForOther = true;
    waitingMessageIndex = size_t(Rand::rand01() * waitingShotMessages.size());
  }
  
  if (myRound <= otherRound) {
    waitingForOther = false;
  }
}

uint32_t MainPhase::gameOver() const {
  if (myBoard.getRemainingHits() == 0)
    return 2;
  if (otherBoard.getRemainingHits() == 0)
    return 1;
  return 0;
}
