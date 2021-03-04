#include "BattleShips.h"

#include "BoardPhase.h"

BoardPhase::BoardPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui boardSize) :
GamePhase(app,gamePhaseID),
boardSize(boardSize)
{}

void BoardPhase::init() {
  GamePhase::init();
  
  Image cellImage{5,5};
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

  for (uint32_t y = 0;y<cellImage.width;++y) {
    for (uint32_t x = 0;x<cellImage.height;++x) {
      cellImage.setNormalizedValue(x, y, 0, 1.0f);
      cellImage.setNormalizedValue(x, y, 1, 1.0f);
      cellImage.setNormalizedValue(x, y, 2, 1.0f);
      cellImage.setNormalizedValue(x, y, 3, 0.5f);
    }
  }
  shotCell = GLTexture2D{cellImage};
  
  for (uint32_t y = 0;y<cellImage.width;++y) {
    for (uint32_t x = 0;x<cellImage.height;++x) {
      cellImage.setNormalizedValue(x, y, 0, 1.0f);
      cellImage.setNormalizedValue(x, y, 1, 1.0f);
      cellImage.setNormalizedValue(x, y, 2, 0.0f);
      cellImage.setNormalizedValue(x, y, 3, 0.5f);
    }
  }
  aimCell = GLTexture2D{cellImage};

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
