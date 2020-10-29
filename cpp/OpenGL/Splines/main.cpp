#include <iostream>
#include <GLApp.h>

class MyGLApp : public GLApp {
public:
  virtual void init() {
    GL(glEnable(GL_CULL_FACE));
    GL(glDisable(GL_DEPTH_TEST));
    GL(glClearColor(1,1,1,1));
  }
  
  virtual void draw() {
    GL(glClear(GL_COLOR_BUFFER_BIT));

    std::vector<float> data{
     -0.9, -0.9, 0, 1,0,0,1,
     -0.9,  0.9, 0, 0,1,0,1,
      0.9,  0.9, 0, 0,0,1,1,
      0.9, -0.9, 0, 0,0,1,1,
    };
    simpleVb.setData(data,7,GL_DYNAMIC_DRAW);

    glDrawArrays(GL_LINE_LOOP, 0, GLsizei(data.size()/7));
  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
