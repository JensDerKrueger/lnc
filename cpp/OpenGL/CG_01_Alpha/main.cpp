#include <GLApp.h>

class MyGLApp : public GLApp {
public:
  
  Image rectRed{10,10};
  Image rectGreen{10,10};
  float angle{0};
  Mat4 windowScale;
  GLTexture2D background;
  
  virtual void init() override {
    glEnv.setTitle("Alpha");
    
    GL(glEnable(GL_DEPTH_TEST));
    
    GL(glEnable(GL_BLEND));
    GL(glBlendEquation(GL_FUNC_ADD));
    GL(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));
    
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
    
    
    for (uint32_t y = 0;y<rectRed.height;++y) {
      for (uint32_t x = 0;x<rectRed.width;++x) {
        rectRed.setNormalizedValue(x,y,0,1.0f);
        rectRed.setNormalizedValue(x,y,1,0.0f);
        rectRed.setNormalizedValue(x,y,2,0.0f);
        rectRed.setNormalizedValue(x,y,3,y/float(rectRed.height));

        rectGreen.setNormalizedValue(x,y,0,0.0f);
        rectGreen.setNormalizedValue(x,y,1,1.0f);
        rectGreen.setNormalizedValue(x,y,2,0.0f);
        rectGreen.setNormalizedValue(x,y,3,1.0f);
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
      
  virtual void animate(double animationTime) override {
    angle = float(animationTime)*10;
  }
  
  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    setDrawTransform(windowScale);
    drawImage(background);

    setDrawTransform(windowScale *
                     Mat4::rotationZ(-angle*3)  *
                     Mat4::translation(-0.5f,-0.5f, -0.5f) *
                     Mat4::scaling(0.2f));
    drawImage(rectGreen);

    setDrawTransform(windowScale *
                     Mat4::rotationZ(angle*2) *
                     Mat4::translation( 0.5f, 0.5f, -0.6f) *
                     Mat4::scaling(0.2f));
    drawImage(rectRed);
  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}
