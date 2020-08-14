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

#include <Tesselation.h>

GLEnv gl{1024,1024,4,"OpenGL Game of Life", true, false, 4, 1, true};
GLTexture2D gridTexture{GL_NEAREST, GL_NEAREST};
GLBuffer vbFullScreenQuad{GL_ARRAY_BUFFER};
GLBuffer ibFullScreenQuad{GL_ELEMENT_ARRAY_BUFFER};
GLArray fullScreenQuadArray;
GLProgram progFullscreenQuad{GLProgram::createFromFile("fullScreenQuadVS.glsl", "fullScreenQuadFS.glsl")};
GLProgram progPlay{GLProgram::createFromFile("fullScreenQuadVS.glsl", "playFS.glsl")};


class Grid2D {
public:
  Grid2D(size_t width, size_t height) :
    width(width),
    height(height),
    data(this->width*this->height) {}
  
  void setData(size_t x, size_t y, bool value) {
    data[index(x,y)] = value;
  }
  
  bool getData(size_t x, size_t y) const {
    return data[index(x,y)];
  }
  
  size_t getHeight() const {return height;}
  size_t getWidth() const {return width;}
  
  size_t countNeighbours(size_t x, size_t y) const {
    size_t count = 0;
    for (int64_t dy = -1;dy<=1;++dy) {
      for (int64_t dx = -1;dx<=1;++dx) {
        if (dx==0 && dy == 0) continue;
        count += getDataCyclic(int64_t(x)+dx, int64_t(y)+dy) ? 1 : 0;
      }
    }
    return count;
  }
  
  
  std::vector<uint8_t> toByteArray() const {
    std::vector<uint8_t> byteData(data.size());
    for (size_t i = 0;i<data.size();++i) {
      byteData[i] = data[i] ? 255 : 0;
    }
    return byteData;
  }
  
private:
  size_t width;
  size_t height;
  std::vector<bool> data;
  
  size_t index(size_t x, size_t y) const {
    return y*width+x;
  }
  
  bool getDataCyclic(int64_t x, int64_t y) const {
    x = (x+width) % width;
    y = (y+height) % height;
    return getData(x,y);
  }
  
  
};

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
      switch (key) {
          case GLFW_KEY_ESCAPE:
              glfwSetWindowShouldClose(window, GL_TRUE);
              break;
      }
  }
}

void init(Grid2D& g) {
  g.setData(g.getWidth()/2+10,g.getHeight()/2+10, true);
  g.setData(g.getWidth()/2+10,g.getHeight()/2+11, true);
  g.setData(g.getWidth()/2+10,g.getHeight()/2+ 9, true);
  g.setData(g.getWidth()/2+ 9,g.getHeight()/2+10, true);
  g.setData(g.getWidth()/2+11,g.getHeight()/2+ 9, true);
  
  gridTexture.setData(g.toByteArray(), g.getWidth(), g.getHeight(), 1);
  Tesselation fullScreenQuad{Tesselation::genRectangle({0,0,0},2,2)};
  vbFullScreenQuad.setData(fullScreenQuad.getVertices(),3);
  ibFullScreenQuad.setData(fullScreenQuad.getIndices());
  fullScreenQuadArray.bind();
  fullScreenQuadArray.connectVertexAttrib(vbFullScreenQuad,progFullscreenQuad,"vPos",3);
  fullScreenQuadArray.connectIndexBuffer(ibFullScreenQuad);
  
  GLint gridTexLocation{progFullscreenQuad.getUniformLocation("gridSampler")};
  progFullscreenQuad.enable();
  progFullscreenQuad.setTexture(gridTexLocation,gridTexture,0);
}

void setColor(float r, float g, float b) {
  const uint32_t index = 16 + uint32_t(r*5) +
                          6 * uint32_t(g*5) +
                         36 * uint32_t(b*5);
  std::cout << "\033[48;5;" << index << "m";
}

void clear() {
  Dimensions dim{gl.getFramebufferSize()};
  glViewport(0, 0, dim.width, dim.height);
  glClearColor(0.0,1.0,0.0,0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void render(const Grid2D& g) {
  clear();
  gridTexture.setData(g.toByteArray(), g.getWidth(), g.getHeight(), 1);

  progFullscreenQuad.enable();
  fullScreenQuadArray.bind();
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
}

void play(const Grid2D& currentGrid, Grid2D& nextGrid) {
  // enable next buffer for writing
  // enable current buffer (texture) for reading
  // enable playShader
  // render quad
  // disable next buffer
  // disable current buffer
  // swap next and current
  
  
  for (size_t y = 0;y<currentGrid.getHeight();++y) {
    for (size_t x = 0;x<currentGrid.getWidth();++x) {
      const size_t n{currentGrid.countNeighbours(x,y)};
      const bool currentCell{currentGrid.getData(x,y)};
      
      if (currentCell)
        nextGrid.setData(x,y, n == 2 || n == 3);
      else
        nextGrid.setData(x,y, n == 3);
    }
  }
}

int main(int argc, char** argv) {
//  gl.setMouseCallbacks(cursorPositionCallback, mouseButtonCallback, scrollCallback);
  gl.setKeyCallback(keyCallback);
  
  const size_t width{512};
  const size_t height{512};
    
  Grid2D currentGrid{width,height};
  Grid2D nextGrid{width,height};
  
  init(currentGrid);
    
  GLEnv::checkGLError("BeforeFirstLoop");
  do {
    render(currentGrid);
    play(currentGrid, nextGrid);
    std::swap(currentGrid, nextGrid);
//    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    GLEnv::checkGLError("endOfFrame");
    gl.endOfFrame();
  } while (!gl.shouldClose());

    
  return EXIT_SUCCESS;
}
