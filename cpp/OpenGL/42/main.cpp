#include <array>
#include <vector>

#include <GLApp.h>
#include <FontRenderer.h>
#include <Rand.h>

#include "YAK42.h"
#include "YAKTerrain.h"
#include "YAKManager.h"

class YAK42App : public GLApp {
public:
  
  YAK42App() :
    GLApp(640,480,1,"YAK42",true,false)
  {
  }
  
  virtual void init() override {
    fe = fr.generateFontEngine();
    GL(glDisable(GL_BLEND));
    GL(glBlendFunc(GL_ONE, GL_ONE));
    GL(glBlendEquation(GL_FUNC_ADD));
    GL(glClearColor(0,0,0,0));
    GL(glClearDepth(1.0f));
    
    GL(glCullFace(GL_BACK));
    GL(glEnable(GL_CULL_FACE));
    GL(glEnable(GL_DEPTH_TEST));
    GL(glDepthFunc(GL_LESS));
    
    const Dimensions dim = glEnv.getFramebufferSize();
    GL(glViewport(0, 0, GLsizei(dim.width), GLsizei(dim.height)));

    const size_t tileCount = 4;
    for (size_t t = 0;t<tileCount;++t) {
      terrain.requestBricks(brickOffset);
      while (!terrain.bricksReady()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      manager.push(terrain.getBricks());
      brickOffset.z += int32_t(terrain.getSize().y)*terrain.getBrickSize().y;
    }
  }
  
  virtual void mouseMove(double xPosition, double yPosition) override {
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width ||
        yPosition < 0 || yPosition > s.height) return;
    mousePos = Vec2{float(xPosition/s.width),float(1.0-yPosition/s.height)};
  }
  
  virtual void mouseButton(int button, int state, int mods, double xPosition,
                           double yPosition) override {
    if (state != GLFW_PRESS) return;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
    } else {
    }
  }
  
  virtual void mouseWheel(double x_offset, double y_offset, double xPosition,
                          double yPosition) override {
  }
  
  
  virtual void resize(int width, int height) override {
    GL(glViewport(0, 0, GLsizei(width), GLsizei(height)));
  }
      
  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_ESCAPE :
          closeWindow();
          break;
      }
    }
  }
  
  virtual void animate(double animationTime) override {
    this->animationTime = float(animationTime);
    
    const Mat4 globalScale = Mat4::scaling(0.005f);
    const Dimensions dim = glEnv.getFramebufferSize();

    const float zNear  = 0.01f;
    const float zFar   = 1000.0f;
    const float fovY   = 45.0f;
    const float aspect = dim.aspect();

    projection = Mat4::perspective(fovY, aspect, zNear, zFar);    
    view       = Mat4::lookAt({0,4,-4+this->animationTime}, {0,0,this->animationTime}, {0,1,0});
    model      = globalScale;
         
    if (terrain.bricksReady()) manager.push(terrain.getBricks());
    
    manager.setProjection(projection);
    manager.setModelView(view*model);

    if (manager.autoPop()) {
      terrain.requestBricks(brickOffset);
      brickOffset.z += int32_t(terrain.getSize().y)*terrain.getBrickSize().y;
    }
  }
  
  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    manager.render();
  }
  
private:
  Vec2 mousePos;
  FontRenderer fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
  std::shared_ptr<FontEngine> fe{nullptr};
  float animationTime;
  
  Mat4 view;
  Mat4 model;
  Mat4 projection;
  
  YAKManager manager;
  
  Vec3i brickOffset;
  YAKTerrain terrain{{120,50}};
  
};

#ifdef _WIN32
#include <Windows.h>
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main(int argc, char** argv) {
#endif
  try {
    YAK42App myApp;
    myApp.run();
  }
  catch (const GLException& e) {
    std::stringstream ss;
    ss << "Insufficient OpenGL Support " << e.what();
#ifndef _WIN32
    std::cerr << ss.str().c_str() << std::endl;
#else
    MessageBoxA(
      NULL,
      ss.str().c_str(),
      "OpenGL Error",
      MB_ICONERROR | MB_OK
    );
#endif
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
