#include <GLApp.h>
#include <Mat4.h>
#include <ArcBall.h>

#include "QVis.h"
#include "MC.h"

class MyGLApp : public GLApp {
public:
  std::vector<float> data;
  QVis q{"bonsai.dat"};
  uint8_t isovalue{40};
  bool wireframe{false};
  bool surfaceChanged{true};
  ArcBall arcball{{512, 512}};
  Mat4 rotation;
  bool leftMouseDown{false};

  virtual void init() override {
    glEnv.setTitle("Marching Cubes demo");
    glEnv.setSync(false);
    GL(glDisable(GL_CULL_FACE));
    GL(glEnable(GL_DEPTH_TEST));
    GL(glClearColor(0,0,0,0));
    
    extractIsosurface();
  }
  
  void extractIsosurface() {
    surfaceChanged = true;
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
  
  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT));
    setDrawProjection(Mat4::perspective(45, glEnv.getFramebufferSize().aspect(), 0.0001f, 100));
    setDrawTransform(Mat4::lookAt({0,0,2},{0,0,0},{0,1,0}) * rotation );
    
    if (surfaceChanged) {
      drawTriangles(data, TrisDrawType::LIST, wireframe, true);
      surfaceChanged = false;
    } else {
      redrawTriangles(wireframe);
    }
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

  virtual void mouseMove(double xPosition, double yPosition) override {
    if (leftMouseDown) {
      const Quaternion q = arcball.drag({uint32_t(xPosition),uint32_t(yPosition)});
      arcball.click({uint32_t(xPosition),uint32_t(yPosition)});
      rotation = q.computeRotation() * rotation;
    }
  }
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      leftMouseDown = state == GLFW_PRESS;
      arcball.click({uint32_t(xPosition),uint32_t(yPosition)});
    }
  }

  virtual void resize(int width, int height) override {
    const Dimensions dim = glEnv.getWindowSize();
    arcball.setWindowSize({dim.width,dim.height});
  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
