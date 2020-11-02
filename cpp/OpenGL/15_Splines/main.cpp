#include <iostream>
#include <GLApp.h>
#include <Vec2.h>
#include <Vec4.h>
#include <Mat4.h>

class MyGLApp : public GLApp {
public:
  
  double sa = 0;
  double ca = 0;
  
  virtual void init() {
    glEnv.setTitle("Spline Demo");
    GL(glDisable(GL_CULL_FACE));
    GL(glDisable(GL_DEPTH_TEST));
    GL(glClearColor(1,1,1,1));
  }
  
  virtual void animate(double animationTime) {
    sa = sin(animationTime);
    ca = cos(animationTime);
  }
  
     Vec2 computePoly(const Vec2& p0, const Vec2& p1,
                      const Vec2& p2, const Vec2& p3,
                      const Mat4& g, float t) {
       
       Vec4 tVec{1,t,t*t,t*t*t};
       Vec4 pX{p0.x(), p1.x(), p2.x(), p3.x()};
       Vec4 pY{p0.y(), p1.y(), p2.y(), p3.y()};
       
       return {Vec4::dot(tVec, g*pX), Vec4::dot(tVec, g*pY) };
     }
     
     void drawPolySegment(const Vec2& p0, const Vec2& p1,
                          const Vec2& p2, const Vec2& p3,
                          const Mat4& g) {
       const size_t maxVal = 100;
       std::vector<float> curve((maxVal+1)*7);
     
       for (size_t i = 0;i<=maxVal;++i) {
         float t = float(i)/float(maxVal);
         
         Vec2 p = computePoly(p0, p1, p2, p3, g, t);
         
         curve[i*7+0] = p.x();
         curve[i*7+1] = p.y();
         curve[i*7+2] = 0.0f;
         
         curve[i*7+3] = 0.0f;
         curve[i*7+4] = 0.0f;
         curve[i*7+5] = 0.0f;
         curve[i*7+6] = 1.0f;
       }
       drawLines(curve, LineDrawType::LD_STRIP);
     }
     
     void drawHermiteSegment(Vec2 p0, Vec2 p1, Vec2 m0, Vec2 m1) {
       Mat4 g{
          1, 0, 0, 0,
          0, 0, 1, 0,
         -3, 3,-2,-1,
          2,-2, 1, 1
       };
       
       
       drawPolySegment(p0,p1,m0,m1,g);
       
       drawPoints({p0.x(),p0.y(),0,1,0,0,0,
                   p0.x()+m0.x(),p0.y()+m0.y(),0,0,0,1,0,
                   p1.x()+m1.x(),p1.y()-m1.y(),0,0,0,1,0,
                   p1.x(),p1.y(),0,1,0,0,0}, 10);
     }
     
   
     
     void drawBezierSegment(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3) {
       Mat4 g{
          1, 0, 0, 0,
         -3, 3, 0, 0,
          3,-6, 3, 0,
         -1, 3,-3, 1
       };
       
       drawPolySegment(p0,p1,p2,p3,g);
       
       drawPoints({p0.x(),p0.y(),0,1,0,0,0,
                   p1.x(),p1.y(),0,0,0,1,0,
                   p2.x(),p2.y(),0,0,0,1,0,
                   p3.x(),p3.y(),0,1,0,0,0}, 10);
     }
  
    
  
  virtual void draw() {
    GL(glClear(GL_COLOR_BUFFER_BIT));

    {
      setDrawTransform(Mat4::translation(0.0f,0.5f,0.0f));
      const Vec2 p0{-0.5,0.0f};
      const Vec2 m0{float(sa)*0.2f,float(ca)*0.2f};
      const Vec2 m1{0.0f,0.2f};
      const Vec2 p1{0.5f,0.0f};
      drawHermiteSegment(p0,p1,m0,m1);
    }
    

    {
      setDrawTransform(Mat4::translation(0.0f,-0.5f,0.0f));
      const Vec2 p0{-0.5,0.0f};
      const Vec2 p1{float(sa)*0.2f-0.5f,float(ca)*0.2f};
      const Vec2 p2{0.5f,0.2f};
      const Vec2 p3{0.5f,0.0f};
      drawBezierSegment(p0,p1,p2,p3);
    }
  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
