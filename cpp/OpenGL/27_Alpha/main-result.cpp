#include <GLApp.h>

class MyGLApp : public GLApp {
public:
  
  Image rect1{10,10};
  Image rect2{10,10};
  float angle{0};
  Mat4 windowScale;
  GLTexture2D background;
  
  virtual void init() override {
    glEnv.setTitle("Alpha");
    GL(glEnable(GL_DEPTH_TEST));

    GL(glEnable(GL_BLEND));
    // back to front
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
 
    //front to back
    //GL(glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
  
    GL(glBlendEquation(GL_FUNC_ADD));

    Image backgroundImage{10,10};
    for (uint32_t y = 0;y<backgroundImage.height;++y) {
      for (uint32_t x = 0;x<backgroundImage.width;++x) {
        const float val = (x+y) % 2 ? 0.7f : 1.0f;
        backgroundImage.setNormalizedValue(x,y,0,val);
        backgroundImage.setNormalizedValue(x,y,1,val);
        backgroundImage.setNormalizedValue(x,y,2,val);
        backgroundImage.setNormalizedValue(x,y,3,1.0f);
      }
    }
    background = GLTexture2D(backgroundImage);
    
    
    for (uint32_t y = 0;y<rect1.height;++y) {
      for (uint32_t x = 0;x<rect1.width;++x) {
        rect1.setNormalizedValue(x,y,0,1.0f);
        rect1.setNormalizedValue(x,y,1,0.0f);
        rect1.setNormalizedValue(x,y,2,0.0f);
        rect1.setNormalizedValue(x,y,3,float(y)/rect1.height);

        rect2.setNormalizedValue(x,y,0,0.0f);
        rect2.setNormalizedValue(x,y,1,1.0f);
        rect2.setNormalizedValue(x,y,2,0.0f);
        rect2.setNormalizedValue(x,y,3,1.0f);
      }
    }
  }
  
  virtual void resize(int width, int height) override {
    const Dimensions s = glEnv.getWindowSize();
    const float ax = 1.0f/float(s.width);
    const float ay = 1.0f/float(s.height);
    const float m = std::max(ax,ay);
    windowScale = Mat4::scaling({ax/m, ay/m, 1.0f});
  }
  
  virtual void mouseMove(double xPosition, double yPosition) override {
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
  }
    
  virtual void animate(double animationTime) override {
    angle = float(animationTime)*10;
  }
  
  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    setDrawTransform(windowScale);
    drawImage(background);
    
    setDrawTransform(Mat4::scaling(0.2f) * Mat4::translation(-0.5f,-0.5f, -0.5f) * Mat4::rotationZ(-angle*3) * windowScale);
    drawImage(rect2);

    setDrawTransform(Mat4::scaling(0.2f) * Mat4::translation( 0.5f, 0.5f, -0.6f) * Mat4::rotationZ(angle*2) * windowScale);
    drawImage(rect1);
    
  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}
