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
  vbFullScreenQuad.setData(fullScreenQuad.getVertices(),3);
  ibFullScreenQuad.setData(fullScreenQuad.getIndices());
  fullScreenQuadArray.bind();
  fullScreenQuadArray.connectVertexAttrib(vbFullScreenQuad,progFullscreenQuad,"vPos",3);
  fullScreenQuadArray.connectIndexBuffer(ibFullScreenQuad);
  
  GL(glDisable(GL_DEPTH_TEST));
}

void render() {
  Dimensions dim{gl.getFramebufferSize()};
  GL(glViewport(0, 0, dim.width, dim.height));
  progFullscreenQuad.enable();
  progFullscreenQuad.setTexture(progFullscreenQuad.getUniformLocation("gridSampler"),gridTextures[current],0);
  fullScreenQuadArray.bind();
  GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0));
  progFullscreenQuad.unsetTexture(0);
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
  framebuffer.unbind();
  progEvolve.unsetTexture(0);
  current = 1-current;
}

int main(int argc, char** argv) {
  gl.setMouseCallbacks(cursorPositionCallback, mouseButtonCallback, scrollCallback);
  gl.setKeyCallback(keyCallback);

  init(512,512);
    
  GLEnv::checkGLError("BeforeFirstLoop");
  do {
    render();
    evolve();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    GLEnv::checkGLError("endOfFrame");
    gl.endOfFrame();
  } while (!gl.shouldClose());
  
  return EXIT_SUCCESS;
}
