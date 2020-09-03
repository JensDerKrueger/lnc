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

GLTexture2D leftEyeTexture{GL_NEAREST, GL_NEAREST};
GLTexture2D rightEyeTexture{GL_NEAREST, GL_NEAREST};


GLTexture3D noiseVolume{GL_NEAREST, GL_NEAREST};
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

GLArray stereoArray;
GLProgram progStereo{GLProgram::createFromFile("stereoVS.glsl", "stereoFS.glsl")};


bool autoRotation = false;
float cursorDepth = 0;
float brushSize = 0.1;
float brushDensity = 0.2;

int vec2bitmap(const std::vector<uint8_t>& bitData) {
  int result = 0;
  for (uint8_t bit : bitData) {
    result += 1<<bit;
  }
  return result;
}

int deathMap = vec2bitmap(std::vector<uint8_t>{0,1,2,3,6,7,8,9});
int birthMap = vec2bitmap(std::vector<uint8_t>{5,12,13});

enum BRUSH_MODE {
  BRUSH_MODE_DEPTH,
  BRUSH_MODE_SIZE,
  BRUSH_MODE_DENSITY,
};

BRUSH_MODE brushMode = BRUSH_MODE_DEPTH;
uint8_t paintState = 0;
float stopT = 0;
float xPositionMouse;
float yPositionMouse;

void clearGrid() {
  size_t gridSize = currentGrid->getDepth();
  currentGrid->setEmpty(gridSize,gridSize,gridSize,1);
}

void genRandomData() {
  size_t gridSize = currentGrid->getDepth();
  std::vector<GLubyte> noiseData(gridSize*gridSize*gridSize);
  for (size_t i = 0;i<noiseData.size();++i) {
    noiseData[i] = GLubyte(Rand::rand01()*256);
  }
  noiseVolume.setData(noiseData, gridSize,gridSize,gridSize,1);
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
      case GLFW_KEY_S:
        loadShader();
        break;
      case GLFW_KEY_SPACE: {
        std::random_device rd{};
        std::mt19937 rng(rd());
        deathMap = std::uniform_int_distribution<int>(0, (1<<28)-1)(rng);
        birthMap = std::uniform_int_distribution<int>(0, (1<<28)-1)(rng);
        clearGrid();
        std::cout << deathMap << " " << birthMap << std::endl;
        break;
      }
      case GLFW_KEY_C:
        clearGrid();
        break;
      case GLFW_KEY_R:
        autoRotation = !autoRotation;
        if (!autoRotation) stopT = glfwGetTime();
        break;
      case GLFW_KEY_LEFT_SHIFT:
        brushMode = BRUSH_MODE_SIZE;
        break;
      case GLFW_KEY_LEFT_ALT:
        brushMode = BRUSH_MODE_DENSITY;
        break;
    }
  }
  if (action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_LEFT_SHIFT:
      case GLFW_KEY_LEFT_ALT:
        brushMode = BRUSH_MODE_DEPTH;
        break;
    }
  }
}

static void sizeCallback(GLFWwindow* window, int width, int height) {
  frontFaceTexture.setEmpty( width, height, 3, true);
  backFaceTexture.setEmpty( width, height, 3, true);
  leftEyeTexture.setEmpty( width, height, 4, false);
  rightEyeTexture.setEmpty( width, height, 4, false);
}

static void cursorPositionCallback(GLFWwindow* window, double xPosition, double yPosition) {
  Dimensions dim{gl.getWindowSize()};
  xPositionMouse = float(std::clamp(xPosition/dim.width,  0.0, 1.0));
  yPositionMouse = float(1.0-std::clamp(yPosition/dim.height, 0.0, 1.0));
}

static void mouseButtonCallback(GLFWwindow* window, int button, int state, int mods) {
  if (state == GLFW_PRESS) {
    paintState = 1;
  } else if (state == GLFW_RELEASE) {
     paintState = 0;
  }
}

static void scrollCallback(GLFWwindow* window, double x_offset, double y_offset) {
  float delta = float(y_offset)/frontFaceTexture.getWidth();
  switch (brushMode) {
    case BRUSH_MODE_DEPTH:
      cursorDepth = std::clamp(cursorDepth + delta, 0.0f, 1.0f);
      break;
    case BRUSH_MODE_SIZE:
      brushSize = brushSize + delta;
      break;
    case BRUSH_MODE_DENSITY:
      brushDensity = brushDensity + delta;
      break;
  }
    
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
    
  genRandomData();
  
  GL(glClearDepth(1.0f));
  GL(glClearColor(0,0,0.5,0));
  GL(glEnable(GL_DEPTH_TEST));
  
  Tesselation fullScreenQuad{Tesselation::genRectangle({0,0,0},2,2)};
  evolveArray.bind();
  vbEvolve.setData(fullScreenQuad.getVertices(),3);
  ibEvolve.setData(fullScreenQuad.getIndices());
  evolveArray.connectVertexAttrib(vbEvolve,progEvolve,"vPos",3);
  evolveArray.connectIndexBuffer(ibEvolve);
  
  stereoArray.bind();
  stereoArray.connectVertexAttrib(vbEvolve,progStereo,"vPos",3);
  stereoArray.connectIndexBuffer(ibEvolve);
}

void renderCube(const Dimensions& dim, const Mat4& mvp, GLTexture2D& eyeTexture) {
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
  
  framebuffer.bind( eyeTexture );
  
  GL(glViewport(0, 0, dim.width, dim.height));
  GL(glClearColor(0,0,0,0));
  GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  progCubeBack.enable();
  progCubeBack.setUniform("cursorPos",Vec2{xPositionMouse,yPositionMouse});
  progCubeBack.setUniform("brushSize",brushSize);
  progCubeBack.setTexture("noise",noiseVolume,3);
  progCubeBack.setUniform("brushDensity",brushDensity);
  progCubeBack.setTexture("frontFaces",frontFaceTexture,0);
  progCubeBack.setTexture("backFaces",backFaceTexture,1);
  progCubeBack.setUniform("cursorDepth",cursorDepth);
  progCubeBack.setTexture("grid",*currentGrid,2);
  progCubeBack.setUniform("MVP", mvp);
  GL(glDrawElements(GL_TRIANGLES, cube.getIndices().size(), GL_UNSIGNED_INT, (void*)0));
  progCubeBack.unsetTexture2D(0);
  progCubeBack.unsetTexture2D(1);
  progCubeBack.unsetTexture3D(2);
  progCubeBack.unsetTexture3D(3);
  
  framebuffer.unbind2D();
  
  GL(glDisable(GL_BLEND));
}

void composeStereoImages() {
  GL(glClearColor(0,0,0.0,0));
  GL(glClear(GL_COLOR_BUFFER_BIT));
  GL(glDisable(GL_CULL_FACE));
  GL(glDisable(GL_DEPTH_TEST));

  progStereo.enable();
  progStereo.setTexture("leftEye",leftEyeTexture,0);
  progStereo.setTexture("rightEye",rightEyeTexture,1);
  stereoArray.bind();
  GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0));
  progStereo.unsetTexture2D(0);
  progStereo.unsetTexture2D(1);
}

void render() {
  Dimensions dim{gl.getFramebufferSize()};
  GL(glEnable(GL_CULL_FACE));

  if (!autoRotation) glfwSetTime(stopT);
  const float animationTime = glfwGetTime();
  
  const Mat4 m{Mat4::rotationX(animationTime*157)*Mat4::rotationY(animationTime*47)};
  
  const StereoMatrices sm = Mat4::stereoLookAtAndProjection({0, 0, 3}, {0, 0, 0}, {0, 1, 0},
                                                      45, dim.aspect(), 0.0001, 100, 3,
                                                      0.04);
  
 

  renderCube(dim, Mat4{m*sm.leftView*sm.leftProj}, leftEyeTexture);
  renderCube(dim, Mat4{m*sm.rightView*sm.rightProj}, rightEyeTexture);
  
  composeStereoImages();
}

void evolve() {
  GL(glDisable(GL_CULL_FACE));
  progEvolve.enable();
  progEvolve.setTexture("gridSampler",*currentGrid,2);
  progEvolve.setTexture("noise",noiseVolume,3);
  progEvolve.setUniform("cursorPos",Vec2{xPositionMouse,yPositionMouse});
  progEvolve.setUniform("brushSize",brushSize);
  progEvolve.setUniform("brushDensity",brushDensity);
  progEvolve.setUniform("paintState",paintState);
  progEvolve.setUniform("cursorDepth",cursorDepth);
  progEvolve.setUniform("deathMap",deathMap);
  progEvolve.setUniform("birthMap",birthMap);
  progEvolve.setTexture("frontFaces",frontFaceTexture,0);
  progEvolve.setTexture("backFaces",backFaceTexture,1);

  evolveArray.bind();
  for (size_t i = 0;i<currentGrid->getDepth();++i) {
    progEvolve.setUniform("gridPos",float(i)/currentGrid->getDepth());
    framebuffer.bind( *nextGrid, i );
    GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0));
  }
  framebuffer.unbind3D();
  progEvolve.unsetTexture2D(0);
  progEvolve.unsetTexture2D(1);
  progEvolve.unsetTexture3D(2);
  progEvolve.unsetTexture3D(3);
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
