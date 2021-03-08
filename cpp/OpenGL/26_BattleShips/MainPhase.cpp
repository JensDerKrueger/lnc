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
  
  titleTex = GLTexture2D(app->fr.render(gameTitle));
  remainingShipsTex = GLTexture2D(app->fr.render("Remaining Ships"));
  homeTitleTex = GLTexture2D(app->fr.render(homeTitles[size_t(Rand::rand01() * homeTitles.size())]));
  guestTitleTex = GLTexture2D(app->fr.render(guestTitles[size_t(Rand::rand01() * guestTitles.size())]));
  
  waitingForOther = false;
  waitingMessageIndex = 0;
    
  myRound = 0;
  otherRound = 0;
  shotsFired.clear();
  shotsResponded.clear();
  shotsReceived.clear();
  shotResults.clear();
  
  remainingShips = ShipPlacement::completePlacement;
}

void MainPhase::updateRemainingShips(uint32_t lastLength) {
  for (size_t i = 0;i<remainingShips.size();++i) {
    if (uint32_t(remainingShips[i]) == lastLength) {
      remainingShips.erase(remainingShips.begin()+int64_t(i));
      break;
    }
  }
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
      
      const float tX = (x+0.5f)/boardSize.x()*2.0f-1.0f;
      const float tY = (y+0.5f)/boardSize.y()*2.0f-1.0f;
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
  
  if (backgroundImage) app->drawRect(Vec4(0,0,0,0.7f));
  
  const Mat4 titleTrans = app->computeImageTransformFixedHeight({titleTex.getWidth(), titleTex.getHeight()}, 0.1f, Vec3{0.f,0.9f,0.0f});
  app->setDrawTransform(titleTrans);
  app->drawImage(titleTex);
  
  const Image subTitle = app->fr.render((sunkShipWithLastShot ? "You sunk one of the ships from " : "You are battling ") + app->getOtherName());
  const Mat4 subTitleTrans = app->computeImageTransformFixedHeight({subTitle.width, subTitle.height}, 0.05f, Vec3{0.0f,-0.85f,0.0f});
  app->setDrawTransform(subTitleTrans);
  app->drawImage(subTitle);
  
  const Mat4 transGuest = app->computeImageTransformFixedHeight({guestTitleTex.getWidth(), guestTitleTex.getHeight()}, 0.07f, Vec3{0.5f,0.7f,0.0f});
  const Mat4 transHome = app->computeImageTransformFixedHeight({homeTitleTex.getWidth(), homeTitleTex.getHeight()}, 0.07f, Vec3{-0.5f,0.7f,0.0f});
  
  app->setDrawTransform(transGuest);
  app->drawImage(guestTitleTex);

  app->setDrawTransform(transHome);
  app->drawImage(homeTitleTex);

  const Mat4 myBoardTrans = app->computeImageTransform(boardSize) * Mat4::scaling(0.6f) * Mat4::translation(-0.5f,0.0f,0.0f);
  app->setDrawTransform(myBoardTrans);
  app->drawLines(gridLines, LineDrawType::LIST, 3);
  drawBoard(myBoard, myBoardTrans, app->getClient()->getAim());

  otherBoardTrans = app->computeImageTransform(boardSize) * Mat4::scaling(0.6f) * Mat4::translation(0.5f,0.0f,0.0f);
  app->setDrawTransform(otherBoardTrans);
  app->drawLines(gridLines, LineDrawType::LIST, 3);
  drawBoard(otherBoard, otherBoardTrans, otherCellPos);
    
  app->setDrawTransform(app->computeImageTransformFixedWidth({remainingShipsTex.getWidth(), remainingShipsTex.getHeight()}, 0.1f, Vec3{0.0f,0.5f,0.0f}));
  app->drawImage(remainingShipsTex);

  Mat4 trans = Mat4::translation({0.85f/boardSize.x()*ShipPlacement::getLongestShipLength()/2.0f,0.4f,0.0f});
  const Mat4 blockScale = Mat4::scaling(0.4f/boardSize.x(),0.4f/boardSize.y(),1.0f);
  for (const ShipSize& s : remainingShips) {
    const uint32_t blockCount{uint32_t(s)};
    for (uint32_t i = 0;i<blockCount;++i) {
      app->setDrawTransform(blockScale * trans * Mat4::translation({-0.85f/boardSize.x()*i,0.0f,0.0f}) * app->computeImageTransform({1,1}) );
      app->drawImage(shipCell);
    }
    trans = trans * Mat4::translation({0.0f,-1.5f/boardSize.y(),0.0f});
  }
  
  if (waitingForOther) {
    const Image prompt = app->fr.render(waitingShotMessages[waitingMessageIndex]);
    const Mat4 transPrompt = app->computeImageTransformFixedWidth({prompt.width, prompt.height}, 0.4f);
    app->setDrawTransform(Mat4{});
    app->drawRect(Vec4(0,0,0,0.7f));
    app->setDrawTransform(transPrompt);
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
      result = myBoard.shipSunk(newShot) ? ShotResult::SUNK : ShotResult::HIT;
    } else {
      otherRound++;
    }
    app->getClient()->sendShotResult(result);
    myBoard.addShot(newShot);
    shotsReceived.push_back(newShot);
  }
  
  if (newShotResults.size() > shotResults.size()) {
    const ShotResult newResult = newShotResults[shotResults.size()];
    const Vec2ui pos = shotsFired[shotResults.size()];
    
    if (newResult == ShotResult::MISS) {
      myRound++;
      otherBoard.addMiss(pos);
    } else {
      otherBoard.addHit(pos);
      if (newResult == ShotResult::SUNK) {
        updateRemainingShips(otherBoard.markAsSunk(pos));
      }
    }
    
    sunkShipWithLastShot = (newResult == ShotResult::SUNK);
    
    shotResults.push_back(newResult);
  }

  if (!waitingForOther && myRound > otherRound) {
    waitingForOther = true;
    waitingMessageIndex = size_t(Rand::rand01() * waitingShotMessages.size());
  }
  
  if (myRound <= otherRound) waitingForOther = false;
}

uint32_t MainPhase::gameOver() const {
  if (myBoard.getRemainingHits() == 0)
    return 2;
  if (otherBoard.getRemainingHits() == 0)
    return 1;
  return 0;
}
