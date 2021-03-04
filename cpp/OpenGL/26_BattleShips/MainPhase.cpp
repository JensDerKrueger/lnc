#include "BattleShips.h"

#include "MainPhase.h"

MainPhase::MainPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui& b) :
BoardPhase(app, gamePhaseID, b)
{}


void MainPhase::mouseMove(double xPosition, double yPosition) {
  BoardPhase::mouseMove(xPosition, yPosition);
    
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

void MainPhase::mouseButton(int button, int state, int mods, double xPosition, double yPosition) {
  BoardPhase::mouseButton(button, state, mods, xPosition, yPosition);

  if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_PRESS) {
    if (otherCellPos.x() < boardSize.x() && otherCellPos.y() < boardSize.y()) {
      // TODO: fire at cell "otherCellPos"
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
      }
      
      if (x == aimCoords.x() && y == aimCoords.y())
        app->drawImage(aimCell);
    }
  }
}


void MainPhase::draw() {
  BoardPhase::draw();
  
  Image prompt = app->fr.render("Home");
  app->setDrawTransform(app->computeImageTransform({prompt.width, prompt.height}) * Mat4::scaling(0.2f) * Mat4::translation(-0.5f,0.8f,0.0f));
  app->drawImage(prompt);

  prompt = app->fr.render("Enemy");
  app->setDrawTransform(app->computeImageTransform({prompt.width, prompt.height}) * Mat4::scaling(0.2f) * Mat4::translation( 0.5f,0.8f,0.0f));
  app->drawImage(prompt);

  
  Mat4 myBoardTrans = app->computeImageTransform(boardSize) * Mat4::scaling(0.6f) * Mat4::translation(-0.5f,0.0f,0.0f);
  app->setDrawTransform(myBoardTrans);
  app->drawLines(gridLines, LineDrawType::LIST, 3);
  
  GameGrid myBoard = app->getMyBoard();
  drawBoard(myBoard, myBoardTrans, app->getClient()->getAim());

  otherBoardTrans = app->computeImageTransform(boardSize) * Mat4::scaling(0.6f) * Mat4::translation(0.5f,0.0f,0.0f);
  app->setDrawTransform(otherBoardTrans);
  app->drawLines(gridLines, LineDrawType::LIST, 3);

  GameGrid otherBoard = app->getOtherBoard();
  drawBoard(otherBoard, otherBoardTrans, otherCellPos);  
}

