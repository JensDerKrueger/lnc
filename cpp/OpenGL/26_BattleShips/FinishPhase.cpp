#include "BattleShips.h"

#include "FinishPhase.h"
#include "Messages.inc"

FinishPhase::FinishPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui& b) :
BoardPhase(app, gamePhaseID, b),
terminate{false}
{
}

void FinishPhase::prepare(const GameGrid& my, const GameGrid& other, const std::string& enc, size_t status) {  
  verification = Verification::Unknown;
  terminate = false;  
  myBoard = my;
  otherBoard = other;
  encOtherBoard = enc;
  
  const std::string title = (status == 1) ? "You win" : "You loose";
  titleTex = GLTexture2D(app->fr.render(title));
  homeTitleTex = GLTexture2D(app->fr.render(homeTitles[size_t(Rand::rand01() * homeTitles.size())]));
  guestTitleTex = GLTexture2D(app->fr.render(guestTitles[size_t(Rand::rand01() * guestTitles.size())]));
}

void FinishPhase::drawBoard(const GameGrid& board, Mat4 boardTrans) {
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
    }
  }
}

void FinishPhase::drawInternal() {
  BoardPhase::drawInternal();
  if (backgroundImage) app->drawRect(Vec4{0,0,0,0.7f});
  
  const Mat4 transGuest = app->computeImageTransformFixedHeight({guestTitleTex.getWidth(), guestTitleTex.getHeight()}, 0.07f, Vec3{0.5f,0.7f,0.0f});
  const Mat4 transHome = app->computeImageTransformFixedHeight({homeTitleTex.getWidth(), homeTitleTex.getHeight()}, 0.07f, Vec3{-0.5f,0.7f,0.0f});
  
  app->setDrawTransform(transGuest);
  app->drawImage(guestTitleTex);

  app->setDrawTransform(transHome);
  app->drawImage(homeTitleTex);

  Mat4 myBoardTrans = app->computeImageTransform(boardSize) * Mat4::scaling(0.6f) * Mat4::translation(-0.5f,0.0f,0.0f);
  app->setDrawTransform(myBoardTrans);
  app->drawLines(gridLines, LineDrawType::LIST, 3);
  drawBoard(myBoard, myBoardTrans);

  Mat4 otherBoardTrans = app->computeImageTransform(boardSize) * Mat4::scaling(0.6f) * Mat4::translation(0.5f,0.0f,0.0f);
  app->setDrawTransform(otherBoardTrans);
  app->drawLines(gridLines, LineDrawType::LIST, 3);
  drawBoard(otherBoard, otherBoardTrans);
  
  app->setDrawTransform(app->computeImageTransform({titleTex.getWidth(), titleTex.getHeight()}) * Mat4::scaling(0.9f));
  app->drawRect(Vec4(0,0,0,0.7f));
  app->setDrawTransform(app->computeImageTransform({titleTex.getWidth(), titleTex.getHeight()}) * Mat4::scaling(0.8f));
  app->drawImage(titleTex);
}

void FinishPhase::animateInternal(double animationTime) {
  BoardPhase::animateInternal(animationTime);
  
  if (verification != Verification::Unknown) return;
  const auto password = app->getClient()->getShipPlacementPassword();
  if (password) {
    if (otherBoard.validate(encOtherBoard, *password)) {
      verification = Verification::Ok;
      ShipPlacement sp = ShipPlacement::fromEncryptedString(encOtherBoard, *password);
      otherBoard.setEnemyShips(sp);
    } else {
      verification = Verification::Invalid;
      titleTex = GLTexture2D(app->fr.render("You win (the other cheated)"));
    }
  }
}

void FinishPhase::mouseButtonInternal(int button, int state, int mods, double xPosition, double yPosition) {
  BoardPhase::mouseButtonInternal(button, state, mods, xPosition, yPosition);
  if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_PRESS) {
    terminate = true;
  }
}
