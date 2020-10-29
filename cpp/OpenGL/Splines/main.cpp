#include <iostream>
#include <GLApp.h>
#include <Vec2.h>

class MyGLApp : public GLApp {
public:
  
  const uint32_t maxA = 120;
  uint32_t a = 0;
  float sa = 0;
  float ca = 0;
  
  virtual void init() {
    GL(glEnable(GL_CULL_FACE));
    GL(glDisable(GL_DEPTH_TEST));
    GL(glClearColor(1,1,1,1));
  }
  
  virtual void animate() {
    a = (a + 1) % (maxA+1);
    sa = sinf(float(a)/maxA*2*M_PI);
    ca = cosf(float(a)/maxA*2*M_PI);
  }
  
  void drawSegment(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3) {
    const size_t maxVal = 100;
    std::vector<float> curve((maxVal+1)*7);
  
    for (size_t i = 0;i<=maxVal;++i) {
      float t = float(i)/float(maxVal);
      
      float b0 = (1.0f-t)*(1.0f-t)*(1.0f-t);
      float b1 = 3.0f*t*(1.0f-t)*(1.0f-t);
      float b2 = 3.0f*t*t*(1.0f-t);
      float b3 = t*t*t;
      
      curve[i*7+0] = p0.x()*b0+p1.x()*b1+p2.x()*b2+p3.x()*b3;
      curve[i*7+1] = p0.y()*b0+p1.y()*b1+p2.y()*b2+p3.y()*b3;
      curve[i*7+2] = 0.0f;
      
      curve[i*7+3] = 0.0;
      curve[i*7+4] = 0.0;
      curve[i*7+5] = 0.0;
      curve[i*7+6] = 1.0f;
    }
    drawLines(curve, LineDrawType::STRIP);
    
    drawPoints({p0.x(),p0.y(),0,1,0,0,0,
                p1.x(),p1.y(),0,0,0,1,0,
                p2.x(),p2.y(),0,0,0,1,0,
                p3.x(),p3.y(),0,1,0,0,0}, 10);
  }
  
  virtual void draw() {
    GL(glClear(GL_COLOR_BUFFER_BIT));

    const Vec2 p0{-0.5,0.0f};
    const Vec2 p1{sa*0.4f-0.5f,ca*0.4f};
    const Vec2 p2{0.5f,0.5f};
    const Vec2 p3{0.5f,0.0f};

    drawSegment(p0,p1,p2,p3);
    
  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
