#include <GLApp.h>
#include <Mat4.h>

#include "Flowfield.h"

class MyGLApp : public GLApp {
public:
  size_t lineCount{20};
  size_t linelength{300};
  double angle{0};
  std::vector<float> data;
  Flowfield flow = Flowfield::genDemo(128, DemoType::SATTLE);
  
  virtual void init() override {
    glEnv.setTitle("Flow Vis Demo 2 (Integral Curves)");
    GL(glDisable(GL_CULL_FACE));
    GL(glEnable(GL_DEPTH_TEST));
    GL(glClearColor(0,0,0,0));
    initLines();
  }

  void initLines() {
    std::vector<Vec3> linePoints;
    linePoints.resize(lineCount*linelength);
    for (size_t l = 0;l<lineCount;++l) {
      linePoints[l*linelength] = Vec3::random();
    }
    
    data.resize(linePoints.size()*2*7);
    
    advect(linePoints, 0.1f);
    linePointsToRenderData(linePoints);
  }
  
  void advect(std::vector<Vec3>& linePoints, double deltaT) {
    for (size_t l = 0;l<lineCount;++l) {
      for (size_t s = 1;s<linelength;++s) {
        const size_t i = l*linelength + s;
        linePoints[i] = advect(linePoints[i-1], deltaT);
        
        if (linePoints[i] == linePoints[j-1]) break;
      }
    }
  }

  Vec3 advect(const Vec3& particle, double deltaT) {
    if (particle.x() < 0.0 || particle.x() > 1.0 ||
        particle.y() < 0.0 || particle.y() > 1.0 ||
        particle.z() < 0.0 || particle.z() > 1.0) {
      return particle;
    }
    return particle + flow.interpolate(particle) * float(deltaT);
  }

  void linePointsToRenderData(const std::vector<Vec3>& linePoints) {
    size_t i = 0;
    for (size_t l = 0;l<lineCount;++l) {
      for (size_t s = 0;s<linelength-1;++s) {
        size_t j = l*linelength + s;
        
        if (linePoints[j] == linePoints[j+1]) break;
        
        data[i++] = linePoints[j].x()*2-1;
        data[i++] = linePoints[j].y()*2-1;
        data[i++] = linePoints[j].z()*2-1;
        
        data[i++] = linePoints[j].x();
        data[i++] = linePoints[j].y();
        data[i++] = linePoints[j].z();
        data[i++] = 1.0f;

        j++;
        
        data[i++] = linePoints[j].x()*2-1;
        data[i++] = linePoints[j].y()*2-1;
        data[i++] = linePoints[j].z()*2-1;
        
        data[i++] = linePoints[j].x();
        data[i++] = linePoints[j].y();
        data[i++] = linePoints[j].z();
        data[i++] = 1.0f;
      }
    }
  }
  
  virtual void animate(double animationTime) override {
    angle = animationTime*10;
  }
  
  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT));
    setDrawProjection(Mat4::perspective(45, glEnv.getFramebufferSize().aspect(), 0.0001f, 100));
    setDrawTransform(Mat4::rotationY(float(angle)) * Mat4::rotationX(float(angle/2.0)) * Mat4::lookAt({0,0,5},{0,0,0},{0,1,0}));
    
    drawLines(data, LineDrawType::LIST, 1.0f);
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
