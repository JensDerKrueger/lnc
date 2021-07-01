#pragma once

#include "LargeImage.h"

#include <GLApp.h>
#include <FontRenderer.h>
#include <Rand.h>
#include <Vec2.h>

class LargeImageRenderer : public GLApp {
public:
  virtual void init() override;
  virtual void mouseMove(double xPosition, double yPosition) override;
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override ;
  virtual void mouseWheel(double x_offset, double y_offset, double xPosition, double yPosition) override;
  virtual void keyboard(int key, int scancode, int action, int mods) override;
  virtual void draw() override;
  
private:
  bool mouseDown{false};
  Vec2 mousePixelPos;
  Vec2 mousePos;
  Vec2 mouseStartPos;
  Mat4 userTransformation;
  LargeImage largeImage{"/Users/lnc/lnc/cpp/OpenGL/37_LargeImageGen/fractal_large.dat",2048};
  FontRenderer fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
  std::shared_ptr<FontEngine> fe{nullptr};
  
  void updateMousePos();
  void addTransformation(const Mat4& trafo);
  void drawInfoText(uint32_t level, const std::vector<TileCoord>& visibleTiles);
  static double log2(double x);
  double computeLevel(const Mat4& drawTransform);
  std::vector<TileCoord> computeVisibleTiles(uint32_t level, const Mat4& drawTransform);
  void renderTiles(const std::vector<TileCoord>& visibleTiles);
  void renderTile(const TileCoord& tileCoord);
  
};

