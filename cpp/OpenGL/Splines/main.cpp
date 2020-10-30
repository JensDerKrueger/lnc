#include <iostream>
#include <GLApp.h>
#include <Vec2.h>

class MyGLApp : public GLApp {
public:
  
  const float PI = 3.1415926535f;
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
    sa = sinf(float(a)/maxA*2*PI);
    ca = cosf(float(a)/maxA*2*PI);
  }
  
  void drawHermiteSegment(Vec2 p0, Vec2 m0, Vec2 m1, Vec2 p1) {
    const size_t maxVal = 100;
    std::vector<float> curve((maxVal+1)*7);
  
    for (size_t i = 0;i<=maxVal;++i) {
      float t = float(i)/float(maxVal);
      
      float h0 = (1.0f-t)*(1.0f-t)*(1.0f+2.0f*t);
      float h1 = t*(1.0f-t)*(1.0f-t);
      float h2 = t*t*(1.0f-t);
      float h3 = (3.0f-2.0f*t)*t*t;
      
      curve[i*7+0] = p0.x()*h0+m0.x()*h1+m1.x()*h2+p1.x()*h3;
      curve[i*7+1] = p0.y()*h0+m0.y()*h1+m1.y()*h2+p1.y()*h3;
      curve[i*7+2] = 0.0f;
      
      curve[i*7+3] = 0.0;
      curve[i*7+4] = 0.0;
      curve[i*7+5] = 0.0;
      curve[i*7+6] = 1.0f;
    }
    drawLines(curve, LineDrawType::STRIP);
    
    drawPoints({p0.x(),p0.y(),0,1,0,0,0,
                p0.x()+m0.x(),p0.y()+m0.y(),0,0,0,1,0,
                p1.x()+m1.x(),p1.y()+m1.y(),0,0,0,1,0,
                p1.x(),p1.y(),0,1,0,0,0}, 10);
  }
  
  void drawBezierSegment(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3) {
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
    const Vec2 p1{sa*0.5f-0.5f,ca*0.5f};
    const Vec2 p2{0.5f,0.5f};
    const Vec2 p3{0.5f,0.0f};

    drawBezierSegment(p0,p1,p2,p3);
  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
