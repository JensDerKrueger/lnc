#include <limits>
#include <math.h>

#include <GLApp.h>

class SignedDistanceTransform : public GLApp {
public:
  Image distanceImage{640,480,4};
  
  Vec2 normPos;
  
  SignedDistanceTransform() :
    GLApp(640,480,1)
  {
  }
  
  virtual void init() override {
    glEnv.setTitle("Signed Distance Transform");
  }
  
  virtual void mouseMove(double xPosition, double yPosition) override{
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
    normPos = Vec2{float(xPosition)/s.width,1.0f-float(yPosition)/s.height};
  }
    
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      if (state == GLFW_PRESS) {
        uint32_t x = uint32_t(normPos.x()*distanceImage.width);
        uint32_t y = uint32_t(normPos.y()*distanceImage.height);
      }
    }
  }

  virtual void draw() override {
    drawImage(distanceImage);
  }
  
} signedDistanceTransform;

int main(int argc, char ** argv) {
  signedDistanceTransform.run();
  return EXIT_SUCCESS;
}
