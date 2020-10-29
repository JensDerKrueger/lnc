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
  
    const float p0 = sinf(float(a)/maxA*2*M_PI);
    const float p1 = 1.0f;
    const float p2 = 1.0f;
    const float p3 = 0.0f;

    for (size_t i = 0;i<=maxVal;++i) {
      float t = float(i)/float(maxVal);
      
      float b0 = (1.0f-t)*(1.0f-t)*(1.0f-t);
      float b1 = 3.0f*t*(1.0f-t)*(1.0f-t);
      float b2 = 3.0f*t*t*(1.0f-t);
      float b3 = t*t*t;

      float st = p0*b0+
                 p1*b1+
                 p2*b2+
                 p3*b3;
      
      curve[i*7+0] = t-0.5f;
      curve[i*7+1] = st;
      curve[i*7+2] = 0.0f;
      
      curve[i*7+3] = 0.0;
      curve[i*7+4] = 0.0;
      curve[i*7+5] = 0.0;
      curve[i*7+6] = 1.0f;
    }
    drawLines(curve, LineDrawType::STRIP);
    
    drawPoints({0-0.5f,p0,0,1,0,0,0,
                1-0.5f,p1,0,1,0,0,0,
                0-0.5f,p2,0,1,0,0,0,
                1-0.5f,p3,0,1,0,0,0}, 10);

  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
