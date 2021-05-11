#include <GLApp.h>
#include <bmp.h>

#include "Flowfield.h"

class MyGLApp : public GLApp {
public:
  Flowfield flow = Flowfield::genDemo(256, DemoType::SATTLE);
  //this field my be a better start for debugging
  //Flowfield flow = Flowfield::fromFile("four_sector_128.txt");
  Image inputImage = BMP::load("noise.bmp");
  Image licImage{uint32_t(flow.getSizeX()),uint32_t(flow.getSizeY()),3};

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
