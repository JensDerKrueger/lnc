#include "BattleShips.h"

#include "FinishPhase.h"
#include "Messages.inc"

FinishPhase::FinishPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui& b) :
BoardPhase(app, gamePhaseID, b),
terminate{false}
{
  homeTitle  = homeTitles[size_t(Rand::rand01() * homeTitles.size())];
  guestTitle = guestTitles[size_t(Rand::rand01() * guestTitles.size())];
}

void FinishPhase::prepare(const GameGrid& my, const GameGrid& other, const std::string& enc, size_t status) {  
  verification = Verification::Unknown;
  terminate = false;
  
  myBoard = my;
  otherBoard = other;
  encOtherBoard = enc;
  
  if (status == 1) {
    title = "You win";
  } else {
    title = "You loose";
  }
}

void FinishPhase::mouseButton(int button, int state, int mods, double xPosition, double yPosition) {
  BoardPhase::mouseButton(button, state, mods, xPosition, yPosition);

  if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_PRESS) {
    terminate = true;
  }
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

void FinishPhase::draw() {
  BoardPhase::draw();
  
  if (backgroundImage) {
    app->setDrawTransform(app->computeImageTransform({backgroundImage->getWidth(), backgroundImage->getHeight()}) );
    app->drawImage(*backgroundImage);
    app->drawRect(Vec4(0,0,0,0.7f));
  }

  
  Image prompt = app->fr.render(homeTitle);
  app->setDrawTransform(app->computeImageTransform({prompt.width, prompt.height}) * Mat4::scaling(0.2f) * Mat4::translation(-0.5f,0.8f,0.0f));
  app->drawImage(prompt);

  prompt = app->fr.render(guestTitle);
  app->setDrawTransform(app->computeImageTransform({prompt.width, prompt.height}) * Mat4::scaling(0.2f) * Mat4::translation( 0.5f,0.8f,0.0f));
  app->drawImage(prompt);

  Mat4 myBoardTrans = app->computeImageTransform(boardSize) * Mat4::scaling(0.6f) * Mat4::translation(-0.5f,0.0f,0.0f);
  app->setDrawTransform(myBoardTrans);
  app->drawLines(gridLines, LineDrawType::LIST, 3);
  
  drawBoard(myBoard, myBoardTrans);

  Mat4 otherBoardTrans = app->computeImageTransform(boardSize) * Mat4::scaling(0.6f) * Mat4::translation(0.5f,0.0f,0.0f);
  app->setDrawTransform(otherBoardTrans);
  app->drawLines(gridLines, LineDrawType::LIST, 3);

  if (verification == Verification::Ok) {
    drawBoard(otherBoard, otherBoardTrans);
  } else {
    drawBoard(otherBoard, otherBoardTrans);
  }
  
  prompt = app->fr.render(title);
  app->setDrawTransform(app->computeImageTransform({prompt.width, prompt.height}) * Mat4::scaling(0.9f));
  app->drawRect(Vec4(0,0,0,0.7f));
  app->setDrawTransform(app->computeImageTransform({prompt.width, prompt.height}) * Mat4::scaling(0.8f));
  app->drawImage(prompt);
}

void FinishPhase::animate(double animationTime) {
  if (verification != Verification::Unknown) return;
  
  const auto password = app->getClient()->getShipPlacementPassword();
  if (password) {
    if (otherBoard.validate(encOtherBoard, *password)) {
      verification = Verification::Ok;
      ShipPlacement sp = ShipPlacement::fromEncryptedString(encOtherBoard, *password);
      otherBoard.setEnemyShips(sp);
    } else {
      verification = Verification::Invalid;
      title = "You win (the other cheated)";
    }
  }
}
