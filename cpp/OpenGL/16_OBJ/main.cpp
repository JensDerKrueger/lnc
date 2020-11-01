#include <iostream>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>

#include <GLApp.h>
#include <OBJFile.h>
#include <Vec2.h>
#include <Vec4.h>
#include <Mat4.h>

class MyGLApp : public GLApp {
public:
  float angle = 0;
  std::vector<float> data;
  
  virtual void init() {
    GL(glDisable(GL_CULL_FACE));
    GL(glEnable(GL_DEPTH_TEST));
    GL(glClearColor(0,0,0,0));
    
    const OBJFile m{"bunny.obj", true};
    for (const OBJFile::IndexType& triangle : m.indices) {
      for (const size_t& index : triangle) {
        data.push_back(m.vertices[index][0]);
        data.push_back(m.vertices[index][1]);
        data.push_back(m.vertices[index][2]);
         
        data.push_back(m.vertices[index][0]+0.5f);
        data.push_back(m.vertices[index][1]+0.5f);
        data.push_back(m.vertices[index][2]+0.5f);
        data.push_back(1.0f);

        data.push_back(m.normals[index][0]);
        data.push_back(m.normals[index][1]);
        data.push_back(m.normals[index][2]);
      }
    }
  }
  
  virtual void animate(double animationTime) {
    angle = animationTime*30;
  }
  
  virtual void draw() {
    GL(glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT));
    setDrawProjection(Mat4::perspective(90, glEnv.getFramebufferSize().aspect(), 0.0001, 100));
    setDrawTransform(Mat4::rotationY(angle) * Mat4::lookAt({0,0,1},{0,0,0},{0,1,0}));
    drawTriangles(data, TrisDrawType::TD_LIST, true);
  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
