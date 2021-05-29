#include <vector>
#include <algorithm>
#include <iostream>
#include <thread>
#include <chrono>

#include <GLEnv.h>
#include <GLTexture2D.h>
#include <GLBuffer.h>
#include <GLArray.h>
#include <GLProgram.h>
#include <GLFramebuffer.h>
#include <Rand.h>

#include <Tesselation.h>

GLEnv gl{1024,1024,4,"OpenGL Game of Life", true, false, 4, 1, true};
std::vector<GLTexture2D> gridTextures{GLTexture2D{GL_NEAREST, GL_NEAREST},
                                      GLTexture2D{GL_NEAREST, GL_NEAREST}};
GLBuffer vbFullScreenQuad{GL_ARRAY_BUFFER};
GLBuffer ibFullScreenQuad{GL_ELEMENT_ARRAY_BUFFER};
GLArray fullScreenQuadArray;
GLProgram progFullscreenQuad{GLProgram::createFromFile("fullScreenQuadVS.glsl", "fullScreenQuadFS.glsl")};

GLFramebuffer framebuffer;
GLProgram progEvolve{GLProgram::createFromFile("fullScreenQuadVS.glsl", "evolveFS.glsl")};
size_t current = 0;
int32_t paintState = 0;
float brushSize = 1.0f;
Vec2 paintPosition{-1,-1};

// torus for visualization
Tesselation torus{Tesselation::genTorus({0, 0, 0}, 0.7f, 0.3f)};
GLBuffer vbTorus{GL_ARRAY_BUFFER};
GLBuffer nbTorus{GL_ARRAY_BUFFER};
GLBuffer txTorus{GL_ARRAY_BUFFER};
GLBuffer ibTorus{GL_ELEMENT_ARRAY_BUFFER};
GLArray torusArray;
GLProgram progTorus{GLProgram::createFromFile("visualizeVS.glsl", "visualizeFS.glsl")};

bool drawTorus{false};

void randomizeGrid() {
  std::vector<uint8_t> data(gridTextures[0].getSize());
  
  for (size_t i = 0;i<data.size();i+=3) {
    data[i] = Rand::rand01() >= 0.5 ? 255 : 0;
  }
  
  gridTextures[0].setData(data);
  gridTextures[1].setData(data);
}

void clearGrid() {
  gridTextures[0].clear();
  gridTextures[1].clear();
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
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
    }
  }
}

void paint(GLFWwindow* window) {
  int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
  
  double xPosition, yPosition;
  glfwGetCursorPos(window, &xPosition, &yPosition);
  
  int xsize, ysize;
  glfwGetWindowSize(window, &xsize, &ysize);
  paintPosition = Vec2{float(xPosition/xsize),1.0f-float(yPosition/ysize)};

  if (state == GLFW_PRESS) {
    paintState = 1;
  } else if (state == GLFW_RELEASE) {
    paintState = 0;
  }
}

static void cursorPositionCallback(GLFWwindow* window, double xPosition, double yPosition) {
  paint(window);
}

static void mouseButtonCallback(GLFWwindow* window, int button, int state, int mods) {
  paint(window);
}

static void scrollCallback(GLFWwindow* window, double x_offset, double y_offset) {
  brushSize = std::max<float>(brushSize + y_offset, 1.0f);
}

void init(const size_t width, const size_t height) {
  gridTextures[0].setEmpty( width, height, 3);
  gridTextures[1].setEmpty( width, height, 3);
  Tesselation fullScreenQuad{Tesselation::genRectangle({0,0,0},2,2)};
  fullScreenQuadArray.bind();
  vbFullScreenQuad.setData(fullScreenQuad.getVertices(),3);
  ibFullScreenQuad.setData(fullScreenQuad.getIndices());
  fullScreenQuadArray.connectVertexAttrib(vbFullScreenQuad,progFullscreenQuad,"vPos",3);
  fullScreenQuadArray.connectIndexBuffer(ibFullScreenQuad);
}

void initTorus() {
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


void render() {
  Dimensions dim{gl.getFramebufferSize()};
  GL(glViewport(0, 0, dim.width, dim.height));
  progFullscreenQuad.enable();
  progFullscreenQuad.setTexture(progFullscreenQuad.getUniformLocation("gridSampler"),gridTextures[current],0);
  fullScreenQuadArray.bind();
  GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0));
  progFullscreenQuad.unsetTexture2D(0);
}

void renderTorus() {
  Dimensions dim{gl.getFramebufferSize()};
  GL(glViewport(0, 0, dim.width, dim.height));
  
  GL(glClearDepth(1.0f));
  GL(glClearColor(0.1f, 0.4f, 0.2f, 1.0f));
  GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  GL(glEnable(GL_DEPTH_TEST));

  const float t0 = glfwGetTime()/5.0;
  const Mat4 m{Mat4::rotationY(t0*47)*
               Mat4::translation({0.8f, 0.0f, 0.0f})*
               Mat4::rotationX(t0*157)*
               Mat4::translation({0.0f, 0.0f, 0.8f})};
  const Mat4 v{Mat4::lookAt({0, 0, 5}, {0, 0, 0}, {0, 1, 0})};
  const Mat4 p{Mat4::perspective(45, dim.aspect(), 0.0001, 100)};
  const Mat4 mvp{p*v*m};

  progTorus.enable();
  progTorus.setUniform(progTorus.getUniformLocation("MVP"), mvp);
  progTorus.setUniform(progTorus.getUniformLocation("M"), m);
  progTorus.setUniform(progTorus.getUniformLocation("Mit"), Mat4::inverse(m), true);
  progTorus.setUniform(progTorus.getUniformLocation("invV"), Mat4::inverse(m));
  Vec3 lightPos{Mat4::rotationY(t0*55)*Vec3{0, 0, 1}};
  progTorus.setUniform(progTorus.getUniformLocation("vLightPos"), lightPos);
  progTorus.setTexture(progTorus.getUniformLocation("textureSampler"),gridTextures[current],0);
  torusArray.bind();

  GL(glDrawElements(GL_TRIANGLES, torus.getIndices().size(), GL_UNSIGNED_INT, (void*)0));
  progTorus.unsetTexture2D(0);
  GL(glDisable(GL_DEPTH_TEST));
}

void evolve() {
  framebuffer.bind( gridTextures[1-current] );
  progEvolve.enable();
  progEvolve.setUniform(progEvolve.getUniformLocation("paintPos"), paintPosition);
  progEvolve.setUniform(progEvolve.getUniformLocation("brushSize"), brushSize);
  progEvolve.setUniform(progEvolve.getUniformLocation("paintState"), paintState);
  progEvolve.setTexture(progEvolve.getUniformLocation("gridSampler"),gridTextures[current],0);
  fullScreenQuadArray.bind();
  GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0));
  framebuffer.unbind2D();
  progEvolve.unsetTexture2D(0);
  current = 1-current;
}

int main(int argc, char** argv) {
  gl.setMouseCallbacks(cursorPositionCallback, mouseButtonCallback, scrollCallback);
  gl.setKeyCallback(keyCallback);

  init(512,512);
  initTorus();
    
  GLEnv::checkGLError("BeforeFirstLoop");
  do {
    if (drawTorus) {
      renderTorus();
    } else {
      render();
    }
    evolve();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    GLEnv::checkGLError("endOfFrame");
    gl.endOfFrame();
  } while (!gl.shouldClose());
  
  return EXIT_SUCCESS;
}
