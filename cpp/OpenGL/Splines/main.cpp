#include <iostream>
#include <GLApp.h>

class MyGLApp : public GLApp {
public:
  
  const uint32_t maxA = 60;
  uint32_t a = 0;
  
  virtual void init() {
    GL(glEnable(GL_CULL_FACE));
    GL(glDisable(GL_DEPTH_TEST));
    GL(glClearColor(1,1,1,1));
  }
  
  virtual void animate() {
    a = (a + 1) % (maxA+1);
  }
  
  virtual void draw() {
    GL(glClear(GL_COLOR_BUFFER_BIT));

  
    const size_t maxVal = 100;
    std::vector<float> curve((maxVal+1)*7);
  
    const float p0 = 0.0f;
    const float p1 = 0.0f;
    const float m0 = sinf(float(a)/maxA*2*M_PI);
    const float m1 = 1.0f;

    for (size_t i = 0;i<=maxVal;++i) {
      float t = float(i)/float(maxVal);
      
      float h0 = (1.0f-t)*(1.0f-t)*(1.0f+2*t);
      float h1 = t*(1.0f-t)*(1.0f-t);
      float h2 = t*t*(1.0f-t);
      float h3 = (3.0f-2.0f*t)*t*t;

      float st = p0*h0+
                 m0*h1+
                 m1*h2+
                 p1*h3;
      
      curve[i*7+0] = t-0.5f;
      curve[i*7+1] = st;
      curve[i*7+2] = 0.0f;
      
      curve[i*7+3] = 0.0;
      curve[i*7+4] = 0.0;
      curve[i*7+5] = 0.0;
      curve[i*7+6] = 1.0f;
    }
    drawLines(curve, LineDrawType::STRIP);

  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
