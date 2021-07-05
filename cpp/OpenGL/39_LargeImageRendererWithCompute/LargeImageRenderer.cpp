#include "LargeImageRenderer.h"

void LargeImageRenderer::init() {
  fe = fr.generateFontEngine();
  glEnv.setTitle("Large Image Renderer");
  GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL(glBlendEquation(GL_FUNC_ADD));
  GL(glEnable(GL_BLEND));
  GL(glClearColor(0,0,0,0));
}

void LargeImageRenderer::mouseMove(double xPosition, double yPosition) {
  const Dimensions s = glEnv.getWindowSize();
  if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
  mousePixelPos = Vec2{float(xPosition), float(yPosition)};
  updateMousePos();
  if (mouseDown) {
    const Vec2 trans = mousePos - mouseStartPos;
    addTransformation(Mat4::translation(trans.x, trans.y, 0));
    mouseStartPos = mousePos;
  }
}

void LargeImageRenderer::updateMousePos() {
  Dimensions s = glEnv.getWindowSize();
  mousePos = Vec2{float(mousePixelPos.x/s.width)-0.5f,float(1.0-mousePixelPos.y/s.height)-0.5f} * 2.0f;
  mousePos = (Mat4::inverse(userTransformation) * Vec4{mousePos,0.0f,1.0f}).xy;
}


void LargeImageRenderer::mouseButton(int button, int state, int mods, double xPosition, double yPosition) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    mouseDown = state == GLFW_PRESS;
    mouseStartPos = mousePos;
  }
}

void LargeImageRenderer::mouseWheel(double x_offset, double y_offset, double xPosition, double yPosition) {
  addTransformation(Mat4::translation(mousePos.x, mousePos.y, 0)*
                    Mat4::scaling(1.0f+float(y_offset)/10) *
                    Mat4::translation(-mousePos.x, -mousePos.y, 0));
}

void LargeImageRenderer::keyboard(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE :
        closeWindow();
        break;
      case GLFW_KEY_R :
        userTransformation = {};
        break;
      case GLFW_KEY_T :
        showTiles = !showTiles;
        break;
      case GLFW_KEY_C :
        largeImage.computeFractal(!largeImage.getComputeFractal());
        break;
    }
  }
}

void LargeImageRenderer::draw() {
  GL(glClear(GL_COLOR_BUFFER_BIT));
  const float scale = 1/glEnv.getFramebufferSize().aspect();
  const Mat4 drawTransform = userTransformation *
                             ((scale < 1) ? Mat4::scaling(scale, 1, 1) : Mat4::scaling(1, 1/scale, 1));
  const double floatLevel = computeLevel(drawTransform);
  const uint32_t level = uint32_t(std::clamp<double>(floatLevel, 0.0, largeImage.getLevelCount()-1));
  std::vector<TileCoord> visibleTiles = computeVisibleTiles(level, drawTransform);  
  setDrawTransform(drawTransform);
  shaderUpdate();
  renderTiles(visibleTiles);
  drawInfoText(level, visibleTiles);
}


void LargeImageRenderer::addTransformation(const Mat4& trafo) {
  userTransformation = userTransformation * trafo;
  updateMousePos();
}

void LargeImageRenderer::drawInfoText(uint32_t level, const std::vector<TileCoord>& visibleTiles) {
  std::stringstream ss;
  ss << "Rendering image at level " << level << " with " << visibleTiles.size();
  if (visibleTiles.size() == 1)
    ss << " active tile out of " << (largeImage.getLevelTiles(level)*largeImage.getLevelTiles(level)) << ".";
  else
    ss << " active tiles out of " << (largeImage.getLevelTiles(level)*largeImage.getLevelTiles(level)) << ".";
  fe->render(ss.str(), glEnv.getFramebufferSize().aspect(), 0.02f, {0,-0.95f});
}

double LargeImageRenderer::log2(double x) {
  return log(x) / log(2);
}

double LargeImageRenderer::computeLevel(const Mat4& drawTransform) {
  const Dimensions s = glEnv.getWindowSize();
  const Vec4 pos{float(s.width),float(s.height),0,0};
  const float domainWidth = (drawTransform * pos).x;
  return log2(largeImage.getInputDim()/domainWidth);
}

std::vector<TileCoord> LargeImageRenderer::computeVisibleTiles(uint32_t level, const Mat4& drawTransform) {
  
  const Mat4 invDrawTransform     = Mat4::inverse(drawTransform);
  const Vec4 bottomLeftScreenPos  = invDrawTransform * Vec4{-1,-1,0,1};
  const Vec4 topRightScreenPos    = invDrawTransform * Vec4{1,1,0,1};
  
  const Vec2 startTileCoordsFloat = (bottomLeftScreenPos.xy+1)/2.0f * (largeImage.getLevelTiles(level)-1);
  const Vec2 endTileCoordsFloat   = (topRightScreenPos.xy+1)/2.0f * (largeImage.getLevelTiles(level)-1);
    
  const Vec2ui startTileCoords{
    uint32_t(std::clamp<float>(floor(startTileCoordsFloat.x), 0.0, largeImage.getLevelTiles(level)-1)),
    uint32_t(std::clamp<float>(floor(startTileCoordsFloat.y), 0.0, largeImage.getLevelTiles(level)-1))
  };
  
  const Vec2ui endTileCoords{
    uint32_t(std::clamp<float>(ceil(endTileCoordsFloat.x), 0.0, largeImage.getLevelTiles(level)-1)),
    uint32_t(std::clamp<float>(ceil(endTileCoordsFloat.y), 0.0, largeImage.getLevelTiles(level)-1))
  };

  std::vector<TileCoord> tileCoords;
  for (uint32_t tileY = startTileCoords.y; tileY <= endTileCoords.y; ++tileY) {
    for (uint32_t tileX = startTileCoords.x; tileX <= endTileCoords.x ; ++tileX) {
      tileCoords.push_back({tileX,tileY,level});
    }
  }
  return tileCoords;
}

void LargeImageRenderer::renderTiles(const std::vector<TileCoord>& visibleTiles) {
  for (const TileCoord& t : visibleTiles) {
    renderTile(t);
  }
}

void LargeImageRenderer::renderTile(const TileCoord& tileCoord) {
  simpleTexProg.enable();

  const float size = 2.0f/(largeImage.getLevelTiles(tileCoord.l));
  const Vec2 bottomLeft{-1.0f + tileCoord.x * size, -1.0f + tileCoord.y * size};
  const Vec2 topRight = bottomLeft + size;
  const float overlap = float(largeImage.getOverlap()) / float(largeImage.getRealTileDim());
  
  std::vector<float> data = {
    topRight.x,      topRight.y, 0.0f, 1.0f-overlap, 1.0f-overlap,
    topRight.x,    bottomLeft.y, 0.0f, 1.0f-overlap, 0.0f+overlap,
    bottomLeft.x,    topRight.y, 0.0f, 0.0f+overlap, 1.0f-overlap,
    bottomLeft.x,    topRight.y, 0.0f, 0.0f+overlap, 1.0f-overlap,
    bottomLeft.x,  bottomLeft.y, 0.0f, 0.0f+overlap, 0.0f+overlap,
    topRight.x,    bottomLeft.y, 0.0f, 1.0f-overlap, 0.0f+overlap
  };
  
  simpleVb.setData(data,5,GL_DYNAMIC_DRAW);
  simpleArray.bind();
  simpleArray.connectVertexAttrib(simpleVb, simpleTexProg, "vPos", 3);
  simpleArray.connectVertexAttrib(simpleVb, simpleTexProg, "vTexCoords", 2, 3);
  
  std::shared_ptr<GLTexture2D> tex = largeImage.getTile(tileCoord);
  simpleTexProg.setTexture("raster",*tex,0);

  GL(glDrawArrays(GL_TRIANGLES, 0, GLsizei(data.size()/5)));
  
  if (showTiles) {
    std::vector<float> coords = {
      topRight.x,      topRight.y, 0.0f, 1,1,1,1,
      topRight.x,    bottomLeft.y, 0.0f, 1,1,1,1,
      bottomLeft.x,  bottomLeft.y, 0.0f, 1,1,1,1,
      bottomLeft.x,    topRight.y, 0.0f, 1,1,1,1,
    };
    drawLines(coords, LineDrawType::LOOP);
  }
  
}

