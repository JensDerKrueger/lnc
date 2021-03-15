#include "BattleShips.h"

#include "BoardPhase.h"

BoardPhase::BoardPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui boardSize) :
GamePhase(app,gamePhaseID),
boardSize(boardSize)
{}

void BoardPhase::init() {
  GamePhase::init();
  
  try {
    emptyCell = GLTexture2D{BMP::load("water.bmp")};
  } catch (const BMP::BMPException& ) {
    emptyCell = GLTexture2D{Image{{0.0f,0.0f,1.0f,1.0f}}};
  }
  
  unknownCell = GLTexture2D{Image{{0.3f,0.3f,0.3f,1.0f}}};
  shipCell = GLTexture2D{Image{{1.0f,0.0f,0.0f,1.0f}}};
  try {
    shotCell = GLTexture2D{BMP::load("hit.bmp")};
  } catch (const BMP::BMPException& ) {
    shotCell = GLTexture2D{Image{{1.0f,1.0f,1.0f,0.5f}}};
  }
    
  try {
    aimCell = GLTexture2D{BMP::load("crosshair.bmp")};
  } catch (const BMP::BMPException& ) {
    aimCell = GLTexture2D{Image{{1.0f,1.0f,0.0f,0.5f}}};
  }
  
  gridLines = gridToLines();
}

std::vector<float> BoardPhase::gridToLines() const {
  std::vector<float> lines;
  
  const uint32_t w = boardSize.x();
  const uint32_t h = boardSize.y();
  
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
