#include <algorithm>

#include "ChatOfLife.h"
#include "golpatterns.h"

std::vector<GOL::Pattern> patternsUsed{
//Still lifes
    //GOL::block,
    //GOL::beehive,
    //GOL::loaf,
    GOL::boat,
    GOL::tub,
//Oscillators
    //GOL::blinker,
    //GOL::toad,
    GOL::beacon,
    GOL::pulsar,
    GOL::pentadecathlon,
//Spaceships
    GOL::glider,
    GOL::lwss,
    GOL::mwss,
    GOL::hwss,
//miscellaneous
    GOL::acorn,
    GOL::gosper
};

ChatOfLife::ChatOfLife() :
GLApp(1024,1024,4,"OpenGL Game of Life with Chat"),
chatConnection("134.91.11.186", 11007)
{
}

ChatOfLife::~ChatOfLife() {
}

void ChatOfLife::randomizeGrid() {
  std::vector<uint8_t> data(gridTextures[0].getSize());
  for (size_t i = 0;i<data.size();i+=3) {
    data[i] = Rand::rand01() >= 0.5 ? 255 : 0;
  }
  gridTextures[0].setData(data);
  gridTextures[1].setData(data);
}
void ChatOfLife::paintBitVector(std::vector<std::vector<uint8_t>>& bits, uint32_t gridX, uint32_t gridY, uint8_t direction, uint8_t value) {
    uint32_t w = gridTextures[0].getWidth();
    uint32_t h = gridTextures[0].getHeight();
    std::vector<std::vector<int8_t>> rot{ {1,0,0,1}, {0,1,-1,0,}, {-1,0,0,-1}, {0,-1,1,0} };

    for (uint32_t yOffset = 0; yOffset < bits.size(); yOffset++) {
        for (uint32_t xOffset = 0; xOffset < bits[yOffset].size(); xOffset++) {
            if (bits[yOffset][xOffset]) {
                uint32_t x{ (w + gridX + rot[direction][0] * xOffset + rot[direction][1] * yOffset) % w };
                uint32_t y{ (h + gridY + rot[direction][2] * xOffset + rot[direction][3] * yOffset) % h };

                gridTextures[0].setPixel({ value,0,0 }, x, y);
                gridTextures[1].setPixel({ value,0,0 }, x, y);
            }
        }
    }
}

std::vector<std::vector<uint8_t>> ChatOfLife::calcRawBitsFromMsg(const std::string &msg) {
  std::vector<std::vector<uint8_t>> bitv{ {} };
  size_t patternWidth = size_t(ceil(sqrt(msg.size() * 8.0f)));
  for (char c : msg) {
    while (c) {
      bitv.back().push_back(c%2);
      if (bitv.back().size() >= patternWidth) { bitv.push_back({}); }
      c /= 2;
    }
  }
  return bitv;
}

std::pair<uint32_t, uint32_t> ChatOfLife::calcPositionFromMsg(const std::string &msg) {
  const char* delimiters = " -_";
  std::string msgT = msg+delimiters[0]+delimiters[0];
  size_t firstLength = { msgT.find_first_of(delimiters)};
  size_t secondLength= { msgT.find_first_of(delimiters,firstLength+1) - firstLength - 1};

  return std::make_pair(uint32_t(firstLength * 47 % gridTextures[0].getWidth()),
                        uint32_t(secondLength * 47 % gridTextures[0].getHeight()));
}

uint8_t ChatOfLife::calcPatternTypeFromMsg(const std::string& msg) {
  return msg.size() > 0 ? msg[0] % 2 : 0;
}

std::vector<std::vector<uint8_t>> ChatOfLife::calcLifeFormFromMsg(const std::string& msg) {
  size_t id = msg.size() > 1 ? uint8_t(msg[1]) % patternsUsed.size() : 0;
  return patternsUsed[id].pattern;
}

uint8_t ChatOfLife::calcDirectionFromMsg(const std::string& msg) {
  return msg.size() > 2 ? msg[2] % 4 : 0;
}

void ChatOfLife::paintPatternByMsg(const std::string &msg) {
  std::vector<std::vector<uint8_t>> bits{};
  if (calcPatternTypeFromMsg(msg) == 0) {
    bits = calcRawBitsFromMsg(msg);
  } else {
    bits = calcLifeFormFromMsg(msg);
  }
  std::pair<uint32_t, uint32_t> gridPos = calcPositionFromMsg(msg);
  uint8_t patternDirection = calcDirectionFromMsg(msg);
  paintBitVector(bits, gridPos.first, gridPos.second, patternDirection, (std::rand()%16)*16);
}

void ChatOfLife::clearGrid() {
  gridTextures[0].clear();
  gridTextures[1].clear();
}

void ChatOfLife::keyboard(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE:
        closeWindow();
        break;
      case GLFW_KEY_R:
        randomizeGrid();
        break;
      case GLFW_KEY_C:
        clearGrid();
        break;
      case GLFW_KEY_T:
        drawTorus = !drawTorus;
        break;
      case GLFW_KEY_SPACE:
      case GLFW_KEY_P:
        paused = !paused;
        break;
      case GLFW_KEY_M:
        std::string msg{};
        std::getline(std::cin, msg);
        paintPatternByMsg(msg);
        break;
    }
  }
}

void ChatOfLife::mouseMove(double xPosition, double yPosition) {
  const Dimensions s = glEnv.getWindowSize();
  paintPosition = Vec2{float(xPosition/s.width),float(1.0-yPosition/s.height)};
}

void ChatOfLife::mouseButton(int button, int state, int mods, double xPosition, double yPosition) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (state == GLFW_PRESS) {
      paintState = 1;
    } else if (state == GLFW_RELEASE) {
      paintState = 0;
    }
  }
}

void ChatOfLife::mouseWheel(double x_offset, double y_offset, double xPosition, double yPosition) {
  brushSize = std::max<float>(brushSize + float(y_offset), 1.0f);
}

void ChatOfLife::init() {
  const Dimensions s = glEnv.getFramebufferSize();
  gridTextures[0].setEmpty( s.width/4, s.height/4, 3);
  gridTextures[1].setEmpty( s.width/4, s.height/4, 3);
  Tesselation fullScreenQuad{Tesselation::genRectangle({0,0,0},2,2)};
  fullScreenQuadArray.bind();
  vbFullScreenQuad.setData(fullScreenQuad.getVertices(),3);
  ibFullScreenQuad.setData(fullScreenQuad.getIndices());
  fullScreenQuadArray.connectVertexAttrib(vbFullScreenQuad,progFullscreenQuad,"vPos",3);
  fullScreenQuadArray.connectIndexBuffer(ibFullScreenQuad);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glDepthFunc(GL_LESS);

  torusArray.bind();
  vbTorus.setData(torus.getVertices(), 3);
  nbTorus.setData(torus.getVertices(), 3);
  txTorus.setData(torus.getTexCoords(), 2);
  ibTorus.setData(torus.getIndices());
  torusArray.connectVertexAttrib(vbTorus, progTorus, "vPos", 3);
  torusArray.connectVertexAttrib(nbTorus, progTorus, "vNorm", 3);
  torusArray.connectVertexAttrib(txTorus, progTorus, "vTc", 2);
  torusArray.connectIndexBuffer(ibTorus);
}

void ChatOfLife::draw() {
  const Dimensions dim{ glEnv.getFramebufferSize() };
  GL(glViewport(0, 0, GLsizei(dim.width), GLsizei(dim.height)));

  if (drawTorus) {
    GL(glClearDepth(1.0f));
    GL(glClearColor(0.1f, 0.4f, 0.2f, 1.0f));
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GL(glEnable(GL_DEPTH_TEST));

    progTorus.enable();
    progTorus.setUniform("MVP", mvp);
    progTorus.setUniform("M", m);
    progTorus.setUniform("Mit", Mat4::inverse(m), true);
    progTorus.setUniform("invV", Mat4::inverse(m));
    progTorus.setUniform("vLightPos", lightPos);
    progTorus.setTexture("textureSampler",gridTextures[current],0);
    torusArray.bind();

    GL(glDrawElements(GL_TRIANGLES, GLsizei(torus.getIndices().size()), GL_UNSIGNED_INT, (void*)0));
    progTorus.unsetTexture2D(0);
    GL(glDisable(GL_DEPTH_TEST));
  } else {
    progFullscreenQuad.enable();
    progFullscreenQuad.setTexture("gridSampler",gridTextures[current],0);
    fullScreenQuadArray.bind();
    GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0));
    progFullscreenQuad.unsetTexture2D(0);
  }
}

void ChatOfLife::animate(double animationTime) {
  framebuffer.bind( gridTextures[1-current] );
  progEvolve.enable();
  progEvolve.setUniform("paintPos", paintPosition);
  progEvolve.setUniform("brushSize", brushSize);
  progEvolve.setUniform("paintState", paintState);
  progEvolve.setTexture("gridSampler",gridTextures[current],0);
  fullScreenQuadArray.bind();
  GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0));
  framebuffer.unbind2D();
  progEvolve.unsetTexture2D(0);
  current = 1-current;
  
  const Dimensions s = glEnv.getWindowSize();
  m = Mat4{Mat4::translation({0.0f, 0.0f, 0.8f})*Mat4::rotationX(float(animationTime*15))*
           Mat4::translation({0.8f, 0.0f, 0.0f})*Mat4::rotationY(float(animationTime*4))};
  const Mat4 v{Mat4::lookAt({0, 0, 5}, {0, 0, 0}, {0, 1, 0})};
  const Mat4 p{Mat4::perspective(45, s.aspect(), 0.0001f, 100)};
  mvp = m*v*p;
  
  lightPos = {Mat4::rotationY(float(animationTime*55))*Vec3{0, 0, 1}};
  
  if (animationTime-lastPingTime > 60) {
    lastPingTime = animationTime;
    chatConnection.sendKeepAlivePing();
  }
  
  std::optional<std::string> pi = chatConnection.getPaintItem();
  if (pi) paintPatternByMsg(*pi);
}
