#include <array>
#include <vector>

#include <GLApp.h>
#include <FontRenderer.h>
#include <Rand.h>

#include "YAK42.h"
#include "YAKManager.h"

class YAK42App : public GLApp {
public:
  
  virtual void init() override {
    fe = fr.generateFontEngine();
    glEnv.setTitle("YAK42");
    GL(glDisable(GL_BLEND));
    GL(glClearColor(0,0,0,0));
    GL(glClearDepth(1.0f));
    
    GL(glCullFace(GL_BACK));
    GL(glEnable(GL_CULL_FACE));
    GL(glEnable(GL_DEPTH_TEST));
    GL(glDepthFunc(GL_LESS));
    
    const int32_t pyraHeight{50};
    const int32_t brickSize{2};
    
    for (int32_t y = 0;y<pyraHeight;++y) {
      const int32_t levelSize{pyraHeight-y};
      const Vec3i startPos{
        -pyraHeight/2*brickSize+brickSize/2+y*brickSize/2,
        -pyraHeight/2,
        -pyraHeight/2*brickSize+brickSize/2+y*brickSize/2
      };
      for (int32_t z = 0;z<levelSize;++z) {
        for (int32_t x = 0;x<levelSize;++x) {
          manager.add(std::make_shared<SimpleYAK42>(brickSize,brickSize,1*3,
                                              Rand::rand<uint16_t>(0,YAK42::colors.size()),
                                              startPos+Vec3i{x*brickSize,y,z*brickSize}));
        }
      }
    }
    manager.compile();
  }
  
  virtual void mouseMove(double xPosition, double yPosition) override {
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
    mousePos = Vec2{float(xPosition/s.width),float(1.0-yPosition/s.height)};
  }
  
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override {
  }
  
  virtual void mouseWheel(double x_offset, double y_offset, double xPosition, double yPosition) override {
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
  }
  
  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
    const Dimensions dim = glEnv.getFramebufferSize();
    
    const Mat4 rotationX = Mat4::rotationX(-60);
    const Mat4 rotationY = Mat4::rotationY(animationTime*60);
    const Mat4 projection{Mat4::perspective(45.0f, dim.aspect(), 0.0001f, 1000.0f)};
    //const Mat4 projection{Mat4::ortho(-3, 3, -3/dim.aspect(), 3/dim.aspect(), 0.0001f, 1000.0f)};

    const Mat4 view = Mat4::lookAt({0,0,3}, {0,0,0}, {0,1,0});
        
    Mat4 model = rotationX*rotationY*globalScale;
    glViewport(0, 0, GLsizei(dim.width), GLsizei(dim.height));
    
    manager.setProjection(projection);
    manager.setModelView(view*model);
    manager.render();
  }
  
private:
  Vec2 mousePos;
  FontRenderer fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
  std::shared_ptr<FontEngine> fe{nullptr};
  float animationTime;
  
  Mat4 globalScale = Mat4::scaling(0.005f);
  
  YAKManager manager;
  
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
