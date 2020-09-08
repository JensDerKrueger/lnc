#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>

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

std::shared_ptr<GLTexture2D> leftEyeTexture = std::make_shared<GLTexture2D>(GL_NEAREST, GL_NEAREST);
std::shared_ptr<GLTexture2D> rightEyeTexture = std::make_shared<GLTexture2D>(GL_NEAREST, GL_NEAREST);


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


enum class RENDER_MODE {
  STANDARD, 
  ANAGLYPH,
};
RENDER_MODE renderMode = RENDER_MODE::STANDARD;

const uint32_t maxNeighbours = 27;
uint32_t neighbourEditPosition = 0;

bool autoRotation = false;
float cursorDepth = 0.2;
float brushSize = 0.1;
float brushDensity = 0.2;
int32_t delay = 0;
uint32_t delayCounter = 0;
float zoom = 3.0f;
float eyeDistance = 0.05f;
float focalDistance = 3.0f;

int vec2bitmap(const std::vector<uint8_t>& bitData) {
  int result = 0;
  for (uint8_t bit : bitData) {
    result += 1<<bit;
  }
  return result;
}
std::string bitmap2vecstring(int map) {
  std::string result = "{";
  int pos = 0;
  while (map) {
    if (map % 2)result += std::to_string(pos) + ",";
    map /= 2;
    pos++;
  }    
  if (pos)result.pop_back();    
  return result+"}";
}


class Rule {
public:
  int birthMap;
  int deathMap;
  
  void save(std::ostream& outStr) {
    outStr.write((char*)&birthMap,sizeof(int));
    outStr.write((char*)&deathMap,sizeof(int));
  }
  
  void load(std::istream& inStr) {
    inStr.read((char*)&birthMap,sizeof(int));
    inStr.read((char*)&deathMap,sizeof(int));
  }

};

std::vector<Rule> rules = {
  { //rare glider, small stable configs, fast die out
    vec2bitmap(std::vector<uint8_t>{5,12,13}), 
    vec2bitmap(std::vector<uint8_t>{0,1,2,3,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27}),
  },
  {256, 268434447}, //chrystal, it collapses if 10 is added to birthMap
  {4352, 268434447} //fireball, slowly growing if seed is big enough
};

void ruleInfo(Rule r) {
  std::cout << "deathMap = " << bitmap2vecstring(r.deathMap) << " " << r.deathMap << std::endl;
  std::cout << "birthMap = " << bitmap2vecstring(r.birthMap) << " " << r.birthMap << std::endl;
}



uint32_t ruleIndex = 0;
Rule rule = rules[ruleIndex];

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

void loadState() {
  std::ifstream inFile{"dump.dat"};




  uint32_t w,h,d;
  inFile.read((char*)&w,sizeof(uint32_t));
  inFile.read((char*)&h,sizeof(uint32_t));
  inFile.read((char*)&d,sizeof(uint32_t));
  std::vector<GLubyte> gridData(w*h*d);
  
  inFile.read((char*)gridData.data(), gridData.size()*sizeof(GLubyte));
  
  if (currentGrid->getWidth() == w &&
      currentGrid->getHeight() == h &&
      currentGrid->getDepth() == d) {
    currentGrid->setData(gridData);
  } else {
    currentGrid->setData(gridData,w,h,d,1);
    nextGrid->setEmpty(w,h,d,1);
  }
  rule.load(inFile);
  inFile.close();
}

void saveState() {
  std::vector<GLubyte> gridData = currentGrid->getDataByte();

  std::ofstream outFile{"dump.dat"};
  
  uint32_t w = currentGrid->getWidth();
  uint32_t h = currentGrid->getHeight();
  uint32_t d = currentGrid->getDepth();
  
  outFile.write((char*)&w,sizeof(uint32_t));
  outFile.write((char*)&h,sizeof(uint32_t));
  outFile.write((char*)&d,sizeof(uint32_t));
  outFile.write((char*)gridData.data(), gridData.size()*sizeof(GLubyte));
  rule.save(outFile);
  outFile.close();
}

static void displayInfo() {
  std::cout << "Keys:" << std::endl;
  std::cout << "    F1/H   Display this Page" << std::endl;
  std::cout << "    ESC    Quit" << std::endl;
  std::cout << "    S      Reload Shaders (evolveVS.glsl, evolveFS.glsl, evolutionRule.glsl)" << std::endl;
  std::cout << "    C      clear the grid" << std::endl;
  std::cout << "    F9     safe" << std::endl;
  std::cout << "    F10    load" << std::endl;
  std::cout << "Paint the Seeds:" << std::endl;
  std::cout << "    left Mousekey        set random cells to be alive" << std::endl;
  std::cout << "    MouseWheel           change the Depth of the seed brush" << std::endl;
  std::cout << "    L_SHIFT+ MouseWheel  change the Size of the seed brush" << std::endl;
  std::cout << "    L_ALT+ MouseWheel    change the Density of the seed brush" << std::endl;
  std::cout << "Rules: cell is dead: birthMap defines at how many neighbours this cell will live" << std::endl;
  std::cout << "       cell is alive: deathMap defines at how many neighbours this cell will die" << std::endl;
  std::cout << "    F5           select the next predefined ruleset" << std::endl;
  std::cout << "    F2           display the active ruleset" << std::endl;
  std::cout << "    SPACE        randomize ruleset (good luck)" << std::endl;
  std::cout << "    LEFT ARROW   select next neighbournumber (doesn't change the ruleset)" << std::endl;
  std::cout << "    RIGHT ARROW  select previous neighbournumber (doesn't change the ruleset)" << std::endl;
  std::cout << "    b            inserts/removes the neighbournumber into/from birthMap" << std::endl;
  std::cout << "    d            inserts/removes the neighbournumber into/from deathMap" << std::endl;
  std::cout << "Visuals:" << std::endl;
  std::cout << "    R         turn auto rotation on/off" << std::endl;
  std::cout << "    1         set Render mode: Standart" << std::endl;
  std::cout << "    2         set Render mode: Anaglyph" << std::endl;
  std::cout << "    3/4       increase/decrease eye distance" << std::endl;
  std::cout << "    5/6       increase/decrease focal distance" << std::endl;
  std::cout << "    PageUP    zoom in" << std::endl;
  std::cout << "    PageDWON  zoom out" << std::endl;
  std::cout << "    +         decrease delay" << std::endl;
  std::cout << "    -         increase delay" << std::endl;
}


static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_H:
      case GLFW_KEY_F1:
        displayInfo();
        break;
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;
      case GLFW_KEY_S:
        loadShader();
        break;
      case GLFW_KEY_SPACE: {
        std::random_device rd{};
        std::mt19937 rng(rd());
        rule.deathMap = std::uniform_int_distribution<int>(0, (1<<(maxNeighbours+1))-1)(rng);
        rule.birthMap = std::uniform_int_distribution<int>(0, (1<<(maxNeighbours+1))-1)(rng);
        clearGrid();
        ruleInfo(rule);
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
      case GLFW_KEY_1:
        renderMode = RENDER_MODE::STANDARD;
        break;
      case GLFW_KEY_2:
        renderMode = RENDER_MODE::ANAGLYPH;
        break;
      case GLFW_KEY_D:
        rule.deathMap = rule.deathMap ^ (1 << neighbourEditPosition);
        std::cout << "deathMap = "<<bitmap2vecstring(rule.deathMap)<<" " << rule.deathMap <<std::endl;
        break;
      case GLFW_KEY_B:
        rule.birthMap = rule.birthMap ^ (1 << neighbourEditPosition);
        std::cout << "birthMap = " << bitmap2vecstring(rule.birthMap) << " " << rule.birthMap << std::endl;
        break;
      case GLFW_KEY_RIGHT:
        neighbourEditPosition = (neighbourEditPosition + 1) % maxNeighbours;
        std::cout << "neighbourEditPosition = " << neighbourEditPosition << std::endl;
        break;
      case GLFW_KEY_LEFT:
        neighbourEditPosition = (neighbourEditPosition + maxNeighbours - 1) % maxNeighbours;
        std::cout << "neighbourEditPosition = " << neighbourEditPosition << std::endl;
        break;
      case GLFW_KEY_F2:
        std::cout << "neighbourEditPosition = " << neighbourEditPosition << std::endl;
        ruleInfo(rule);
        break;
      case GLFW_KEY_F9:
        saveState();
        break;
      case GLFW_KEY_F10:
        loadState();
        break;
      case GLFW_KEY_F5:
        ruleIndex = (ruleIndex + 1) % rules.size();
        rule = rules[ruleIndex];
        ruleInfo(rule);
        break;
      case GLFW_KEY_RIGHT_BRACKET:   // US key for german +
        delay -= 10;
        break;
      case GLFW_KEY_SLASH:           // US key for german -
        delay += 10;
        break;
      case GLFW_KEY_3:
        eyeDistance += 0.001;
        std::cout << "eyeDistance = " << eyeDistance << std::endl;
        break;
      case GLFW_KEY_4:
        eyeDistance -= 0.001;
        std::cout << "eyeDistance = " << eyeDistance << std::endl;
        break;
      case GLFW_KEY_5:
        focalDistance += 0.001;
        std::cout << "focalDistance = " << focalDistance << std::endl;
        break;
      case GLFW_KEY_6:
        focalDistance -= 0.001;
        std::cout << "focalDistance = " << focalDistance << std::endl;
        break;
      case GLFW_KEY_PAGE_UP :
        zoom -= 1;
        break;
      case GLFW_KEY_PAGE_DOWN :
        zoom += 1;
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
  leftEyeTexture->setEmpty( width, height, 4, false);
  rightEyeTexture->setEmpty( width, height, 4, false);
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

void renderInternal(const Dimensions& dim, const Mat4& mvp, std::shared_ptr<GLTexture2D> eyeTexture = nullptr) {
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
  
  if (eyeTexture) framebuffer.bind( *eyeTexture );
  
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
  
  if (eyeTexture) framebuffer.unbind2D();
  
  GL(glDisable(GL_BLEND));
}

void composeStereoImages() {
  GL(glClearColor(0,0,0.0,0));
  GL(glClear(GL_COLOR_BUFFER_BIT));
  GL(glDisable(GL_CULL_FACE));
  GL(glDisable(GL_DEPTH_TEST));

  progStereo.enable();
  progStereo.setTexture("leftEye",*leftEyeTexture,0);
  progStereo.setTexture("rightEye",*rightEyeTexture,1);
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

  switch (renderMode) {
    case RENDER_MODE::STANDARD: {
      const Mat4 v{ Mat4::lookAt({ 0, 0, zoom }, { 0, 0, 0 }, { 0, 1, 0 })};
      const Mat4 p{ Mat4::perspective(45, dim.aspect(), 0.0001, 100) };
      renderInternal(dim, Mat4{ m * v * p });
      break;
    }
    case RENDER_MODE::ANAGLYPH: {
      const StereoMatrices sm = Mat4::stereoLookAtAndProjection({ 0, 0, zoom }, { 0, 0, 0 }, { 0, 1, 0 },
                                                                45, dim.aspect(), 0.0001, 100, focalDistance,
                                                                eyeDistance);
      renderInternal(dim, Mat4{ m * sm.leftView * sm.leftProj }, leftEyeTexture);
      renderInternal(dim, Mat4{ m * sm.rightView * sm.rightProj }, rightEyeTexture);

      composeStereoImages();
      break;
    }
  }

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
  progEvolve.setUniform("deathMap",rule.deathMap);
  progEvolve.setUniform("birthMap",rule.birthMap);
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
  displayInfo();
  GLEnv::checkGLError("BeforeFirstLoop");  
  do {
    render();
    
    if (delayCounter >= delay) {
      evolve();
      delayCounter = 0;
    } else {
      delayCounter++;
    }
    GLEnv::checkGLError("endOfFrame");
    gl.endOfFrame();
  } while (!gl.shouldClose());
  
  return EXIT_SUCCESS;
}
