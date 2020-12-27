#include <GLApp.h>
#include <Mat4.h>

#include "QVis.h"
#include "MC.h"

class MyGLApp : public GLApp {
public:
  double angle{0};
  std::vector<float> data;
  QVis q{"c60.dat"};
  uint8_t isovalue{128};
  bool wireframe{false};
  
  virtual void init() override {
    glEnv.setTitle("Marching Cubes demo");
    GL(glDisable(GL_CULL_FACE));
    GL(glEnable(GL_DEPTH_TEST));
    GL(glClearColor(0,0,0,0));
    
    extractIsosurface();
  }
  
  void extractIsosurface() {
    Isosurface s{q.volume,isovalue};
    data.clear();
    for (const Vertex& v : s.vertices) {
      data.push_back(v.position[0]);
      data.push_back(v.position[1]);
      data.push_back(v.position[2]);
       
      data.push_back(v.position[0]+0.5f);
      data.push_back(v.position[1]+0.5f);
      data.push_back(v.position[2]+0.5f);
      data.push_back(1.0f);

      data.push_back(v.normal[0]);
      data.push_back(v.normal[1]);
      data.push_back(v.normal[2]);
    }
  }
  
  virtual void animate(double animationTime) override {
    angle = animationTime*30;
  }
  
  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT));
    setDrawProjection(Mat4::perspective(45, glEnv.getFramebufferSize().aspect(), 0.0001f, 100));
    setDrawTransform(Mat4::rotationY(float(angle)) * Mat4::rotationX(float(angle/2.0)) * Mat4::lookAt({0,0,2},{0,0,0},{0,1,0}));
    drawTriangles(data, TrisDrawType::LIST, wireframe, true);
  }
  
  virtual void keyboard(int key, int scancode, int action, int mods) override {

    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_ESCAPE:
          closeWindow();
          break;
        case GLFW_KEY_W:
          wireframe = !wireframe;
          break;
      }
    }
    switch (key) {
      case GLFW_KEY_UP:
        isovalue++;
        extractIsosurface();
        break;
      case GLFW_KEY_DOWN:
        isovalue--;
        extractIsosurface();
        break;
    }
  }


} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
