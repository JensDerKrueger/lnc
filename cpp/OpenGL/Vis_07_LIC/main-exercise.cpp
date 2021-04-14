#include <GLApp.h>
#include <bmp.h>

#include "Flowfield.h"

class MyGLApp : public GLApp {
public:
  const uint32_t fieldSize{256};
  Flowfield flow = Flowfield::genDemo(fieldSize, DemoType::SATTLE);
  Image inputImage = BMP::load("noise.bmp");
  Image licImage{fieldSize,fieldSize,4};

  virtual void init() override {
    glEnv.setTitle("LIC demo");
    GL(glDisable(GL_CULL_FACE));
    GL(glDisable(GL_DEPTH_TEST));
    GL(glClearColor(0,0,0,0));
    computeLIC();
  }
  
  void computeLIC() {
    // TODO: perform LIC here and store output in licImage
    licImage = inputImage;
  }
  
  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT));
    drawImage(licImage);
  }
  
  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_ESCAPE:
          closeWindow();
          break;
      }
    }
  }
} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
