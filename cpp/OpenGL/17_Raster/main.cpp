#include <GLApp.h>
#include <Mat4.h>

class MyGLApp : public GLApp {
public:
  Image image{640,480};
  
  uint8_t mandelbrot(size_t x, size_t y) {
    const float dx = 2.8f / image.width, dy = 2.6f / image.height;
    const float xs = -2.1f, ys = -1.3f;
    const float cR = xs + dx*x, cI = ys + dy*y;
    float dR = 0, dI = 0;
    uint16_t depth = 0;
    while (depth < 256 && dR*dR+dI*dI < 4) {
      float t = dR;
      dR = dR*dR-dI*dI+cR;
      dI = 2*t*dI+cI;
      ++depth;
    }
    return uint8_t(depth);
  }
  
  virtual void init() {
    glEnv.setTitle("Raster Demo");
    GL(glDisable(GL_CULL_FACE));
    GL(glClearColor(0,0,0,0));

    for (uint32_t y = 0;y<image.height;++y) {
      for (uint32_t x = 0;x<image.width;++x) {
        uint8_t d = mandelbrot(x,y);
        image.setValue(x,y,0,uint8_t(d));
        image.setValue(x,y,1,uint8_t(d*3));
        image.setValue(x,y,2,uint8_t(d*5));
        image.setValue(x,y,3,255);
      }
    }
  }
    
  virtual void draw() {
    GL(glClear(GL_COLOR_BUFFER_BIT));
    drawImage(image);
  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}  
