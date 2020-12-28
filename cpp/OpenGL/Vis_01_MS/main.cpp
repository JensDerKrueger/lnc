#include <GLApp.h>
#include <bmp.h>
#include "MS.h"

class MyGLApp : public GLApp {
public:
  std::vector<float> data;
  Image image = BMP::load("image.bmp");
  uint8_t isovalue{128};
  bool useAsymptoticDecider{true};
  
  virtual void init() override {
    glEnv.setTitle("Marching Squares demo");
    GL(glDisable(GL_CULL_FACE));
    GL(glDisable(GL_DEPTH_TEST));
    GL(glClearColor(0,0,0,0));
    extractIsoline();
  }
  
  void extractIsoline() {
    Isoline s{image, isovalue, useAsymptoticDecider};
    data.clear();
    for (const Vec2& v : s.vertices) {
      data.push_back(v[0]);
      data.push_back(v[1]);
      data.push_back(0);
       
      data.push_back(0.0f);
      data.push_back(0.0f);
      data.push_back(1.0f);
      data.push_back(1.0f);
    }
  }
  
  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT));
    drawImage(image);
    drawLines(data, LineDrawType::LIST);
  }
  
  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_ESCAPE:
          closeWindow();
          break;
        case GLFW_KEY_D:
          useAsymptoticDecider = ! useAsymptoticDecider;
          std::cout << "Asymptotic Decider is " << (useAsymptoticDecider ? "enabled" : "disabled") << std::endl;
          extractIsoline();
          break;
      }
    }
    switch (key) {
      case GLFW_KEY_UP:
        isovalue++;
        extractIsoline();
        break;
      case GLFW_KEY_DOWN:
        isovalue--;
        extractIsoline();
        break;
    }
  }
} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
