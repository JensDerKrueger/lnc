#include <vector>
#include <algorithm>
#include <iostream>
#include <thread>
#include <chrono>

#include <GLEnv.h>
#include <GLTexture2D.h>
#include <GLTexture3D.h>
#include <GLBuffer.h>
#include <GLArray.h>
#include <GLProgram.h>
#include <GLFramebuffer.h>
#include <Rand.h>
#include <bmp.h>

#include <Tesselation.h>

GLEnv gl{1024,1024,1,"OpenGL Game of Life 3D", true, false, 4, 1, true};

GLFramebuffer framebuffer;
GLTexture2D frontFaceTexture{GL_NEAREST, GL_NEAREST};
GLTexture2D backFaceTexture{GL_NEAREST, GL_NEAREST};

std::shared_ptr<GLTexture3D> currentGrid = std::make_shared<GLTexture3D>(GL_NEAREST, GL_NEAREST);
std::shared_ptr<GLTexture3D> nextGrid = std::make_shared<GLTexture3D>(GL_NEAREST, GL_NEAREST);

Tesselation cube{Tesselation::genBrick({0, 0, 0}, {1, 1, 1})};
GLBuffer vbCube{GL_ARRAY_BUFFER};
GLBuffer ibCube{GL_ELEMENT_ARRAY_BUFFER};
GLArray cubeArray;
GLProgram progCubeFront{GLProgram::createFromFile("cubeVS.glsl", "frontFS.glsl")};
GLProgram progCubeBack{GLProgram::createFromFile("cubeVS.glsl", "backFS.glsl")};

GLBuffer vbEvolve{GL_ARRAY_BUFFER};
GLBuffer ibEvolve{GL_ELEMENT_ARRAY_BUFFER};
GLArray evolveArray;
GLProgram progEvolve{GLProgram::createFromFiles(std::vector<std::string>{"evolveVS.glsl"},
                                                std::vector<std::string>{"evolveFS.glsl", "evolutionRule.glsl"} )};

float cursorDepth = 0;
float brushSize = 0.1;
bool brushSizeMode = false;
uint8_t paintState = 0;
float stopT = 0;
float xPositionMouse;
float yPositionMouse;


void genRandomGrid() {
  size_t gridSize = currentGrid->getDepth();
  std::vector<GLubyte> dummy(gridSize*gridSize*gridSize);
  for (size_t i = 0;i<dummy.size();++i) {
    float x = (i % gridSize) / float(gridSize);
    float y = ((i / gridSize) % gridSize) / float(gridSize);
    float z = (i / (gridSize*gridSize)) / float(gridSize);
    
    float dist = ((x-0.5f)*(x-0.5f) + (y-0.5f)*(y-0.5f) + (z-0.5f)*(z-0.5f));
    
    dummy[i] = dist < 0.1 && Rand::rand01() >= 0.9 ? 255 : 0;
  }
  currentGrid->setData(dummy);
}


void loadShader() {
  progEvolve = GLProgram::createFromFiles(std::vector<std::string>{"evolveVS.glsl"},
                                          std::vector<std::string>{"evolveFS.glsl", "evolutionRule.glsl"});
  evolveArray.bind();
  evolveArray.connectVertexAttrib(vbEvolve,progEvolve,"vPos",3);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;
      case GLFW_KEY_R:
        genRandomGrid();
        break;
      case GLFW_KEY_S:
        loadShader();
        break;
      case GLFW_KEY_LEFT_SHIFT:
        brushSizeMode = true;
        break;
    }
  }
  if (action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_LEFT_SHIFT:
        brushSizeMode = false;
        break;
    }
  }
}

static void sizeCallback(GLFWwindow* window, int width, int height) {
  frontFaceTexture.setEmpty( width, height, 3, true);
  backFaceTexture.setEmpty( width, height, 3, true);
}

static void cursorPositionCallback(GLFWwindow* window, double xPosition, double yPosition) {
  Dimensions dim{gl.getWindowSize()};
  xPositionMouse = float(std::clamp(xPosition/dim.width,  0.0, 1.0));
  yPositionMouse = float(1.0-std::clamp(yPosition/dim.height, 0.0, 1.0));
}

static void mouseButtonCallback(GLFWwindow* window, int button, int state, int mods) {
  if (state == GLFW_PRESS) {
    paintState = 1;
    stopT = glfwGetTime();
  } else if (state == GLFW_RELEASE) {
     paintState = 0;
  }
}

static void scrollCallback(GLFWwindow* window, double x_offset, double y_offset) {
  float delta = float(y_offset)/frontFaceTexture.getWidth();
  
  if (brushSizeMode)
    brushSize = brushSize + delta;
  else
    cursorDepth = std::clamp(cursorDepth + delta, 0.0f, 1.0f);
}

void init() {
  GL(glEnable(GL_CULL_FACE));
  GL(glDisable(GL_DEPTH_TEST));

  cubeArray.bind();
  vbCube.setData(cube.getVertices(), 3);
  ibCube.setData(cube.getIndices());
  cubeArray.connectVertexAttrib(vbCube, progCubeFront, "vPos", 3);
  cubeArray.connectIndexBuffer(ibCube);

  Dimensions dim{gl.getFramebufferSize()};
  sizeCallback(nullptr, dim.width, dim.height);
  
  size_t gridSize{128};
  currentGrid->setEmpty(gridSize,gridSize,gridSize,1);
  nextGrid->setEmpty(gridSize,gridSize,gridSize,1);
  
  genRandomGrid();
  
  GL(glClearDepth(1.0f));
  GL(glClearColor(0,0,0.5,0));
  GL(glEnable(GL_DEPTH_TEST));
  
  Tesselation fullScreenQuad{Tesselation::genRectangle({0,0,0},2,2)};
  evolveArray.bind();
  vbEvolve.setData(fullScreenQuad.getVertices(),3);
  ibEvolve.setData(fullScreenQuad.getIndices());
  evolveArray.connectVertexAttrib(vbEvolve,progEvolve,"vPos",3);
  evolveArray.connectIndexBuffer(ibEvolve);
}

void render() {
  Dimensions dim{gl.getFramebufferSize()};
  GL(glEnable(GL_CULL_FACE));

  if (paintState == 1) {
    glfwSetTime(stopT);
  }
  const float animationTime = glfwGetTime();
  
  const Mat4 m{Mat4::rotationX(animationTime*157)*Mat4::rotationY(animationTime*47)};
  const Mat4 v{Mat4::lookAt({0, 0, 3}, {0, 0, 0}, {0, 1, 0})};
  const Mat4 p{Mat4::perspective(45, dim.aspect(), 0.0001, 100)};
  const Mat4 mvp{m*v*p};

  GL(glCullFace(GL_BACK));
  framebuffer.bind( frontFaceTexture );
  GL(glClearColor(0,0,0.0,0));
  GL(glClear(GL_COLOR_BUFFER_BIT));

  progCubeFront.enable();
  progCubeFront.setUniform(progCubeFront.getUniformLocation("MVP"), mvp);
  cubeArray.bind();
  GL(glDrawElements(GL_TRIANGLES, cube.getIndices().size(), GL_UNSIGNED_INT, (void*)0));
  
  GL(glCullFace(GL_FRONT));
  framebuffer.bind( backFaceTexture );
  GL(glClear(GL_COLOR_BUFFER_BIT));
  GL(glDrawElements(GL_TRIANGLES, cube.getIndices().size(), GL_UNSIGNED_INT, (void*)0));
  framebuffer.unbind2D();

  GL(glEnable(GL_BLEND));
  GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL(glBlendEquation(GL_FUNC_ADD));
  
  GL(glViewport(0, 0, dim.width, dim.height));
  GL(glClearColor(0,0,0.5,0));
  GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  progCubeBack.enable();
  progCubeBack.setUniform(progCubeBack.getUniformLocation("cursorPos"),Vec2{xPositionMouse,yPositionMouse});
  progCubeBack.setUniform(progCubeBack.getUniformLocation("brushSize"),brushSize);
  progCubeBack.setTexture(progCubeBack.getUniformLocation("frontFaces"),frontFaceTexture,0);
  progCubeBack.setTexture(progCubeBack.getUniformLocation("backFaces"),backFaceTexture,1);
  progCubeBack.setUniform(progCubeBack.getUniformLocation("cursorDepth"),cursorDepth);
  progCubeBack.setTexture(progCubeBack.getUniformLocation("grid"),*currentGrid,2);
  progCubeBack.setUniform(progCubeBack.getUniformLocation("MVP"), mvp);
  GL(glDrawElements(GL_TRIANGLES, cube.getIndices().size(), GL_UNSIGNED_INT, (void*)0));
  progCubeBack.unsetTexture2D(0);
  progCubeBack.unsetTexture2D(1);
  progCubeBack.unsetTexture3D(2);
  
  GL(glDisable(GL_BLEND));
}

void evolve() {
  GL(glDisable(GL_CULL_FACE));
  progEvolve.enable();
  progEvolve.setTexture(progEvolve.getUniformLocation("gridSampler"),*currentGrid,2);
  progEvolve.setUniform(progEvolve.getUniformLocation("cursorPos"),Vec2{xPositionMouse,yPositionMouse});
  progEvolve.setUniform(progEvolve.getUniformLocation("brushSize"),brushSize);
  progEvolve.setUniform(progEvolve.getUniformLocation("cursorDepth"),cursorDepth);
  progEvolve.setTexture(progEvolve.getUniformLocation("frontFaces"),frontFaceTexture,0);
  progEvolve.setTexture(progEvolve.getUniformLocation("backFaces"),backFaceTexture,1);

  evolveArray.bind();
  for (size_t i = 0;i<currentGrid->getDepth();++i) {
    progEvolve.setUniform(progEvolve.getUniformLocation("gridPos"),float(i)/currentGrid->getDepth());
    framebuffer.bind( *nextGrid, i );
    GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0));
  }
  framebuffer.unbind3D();
  progEvolve.unsetTexture2D(0);
  progEvolve.unsetTexture2D(1);
  progEvolve.unsetTexture3D(2);
  std::swap(currentGrid, nextGrid);
}

int main(int argc, char** argv) {
  gl.setMouseCallbacks(cursorPositionCallback, mouseButtonCallback, scrollCallback);
  gl.setKeyCallback(keyCallback);
  gl.setResizeCallback(sizeCallback);

  init();
  GLEnv::checkGLError("BeforeFirstLoop");
  do {
    render();
    evolve();
    GLEnv::checkGLError("endOfFrame");
//    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    gl.endOfFrame();
  } while (!gl.shouldClose());
  
  return EXIT_SUCCESS;
}
