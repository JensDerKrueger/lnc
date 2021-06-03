#include <GLApp.h>
#include <FontRenderer.h>
#include <Rand.h>

#include "MosaicMaker.h"

class MyGLApp : public GLApp {
public:
  virtual void init() override{
    glEnv.setTitle("Mosaic Maker");
    fe = fr.generateFontEngine();
    mm.generateAsync();
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL(glBlendEquation(GL_FUNC_ADD));
    GL(glEnable(GL_BLEND));
    GL(glClearColor(0,0,0,0));
  }
  
  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_Q :
          closeWindow();
          break;
      }
    }
  }
  
  virtual void animate(double animationTime) override {
    if (!hasData) {
      if (mm.getProgress().complete) {
        BMP::save("result.bmp", mm.getResultImage());
        
        result = GLTexture2D(mm.getResultImage(2048),
                             GL_LINEAR, GL_LINEAR,
                             GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        
        imageAspect = float(mm.getResultImage().width) /
                      float(mm.getResultImage().height);
        hasData = true;
      }
    }
  }
  
  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT));
    
    if (hasData) {
      float scale = imageAspect/glEnv.getFramebufferSize().aspect();
      if (scale < 1)
        setDrawTransform(Mat4::scaling(scale, 1, 1));
      else
        setDrawTransform(Mat4::scaling(1, 1/scale, 1));
      drawImage(result);
    } else {
      std::stringstream ss;
      if (mm.getProgress().targetCount > 1) {
        ss << mm.getProgress().stageName << " " << (mm.getProgress().currentElement*100 / mm.getProgress().targetCount) << " %";
      } else {
        ss << mm.getProgress().stageName;
      }
      fe->render(ss.str(), glEnv.getFramebufferSize().aspect(), 0.05f, {0,0});
    }
  }
  
private:
  MosaicMaker mm{"/Users/lnc/lnc/cpp/OpenGL/34_Mosaic/smallImages",
                 "/Users/lnc/lnc/cpp/OpenGL/34_Mosaic/jens.bmp",
                 200, {20,20}, {4,7}, {1.5,1.0,1.0}, 0.5};
  FontRenderer fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
  std::shared_ptr<FontEngine> fe{nullptr};
  GLTexture2D result;
  float imageAspect;
  bool hasData{false};

};

#ifdef _WIN32
#include <Windows.h>
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main(int argc, char** argv) {
#endif
  try {
    MyGLApp myApp;
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
