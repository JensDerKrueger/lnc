#include <GLApp.h>

class MyGLApp : public GLApp {
public:
  Image image{640,480};

  Vec3 convertPosToHSV(float x, float y) {
    // TODO:
    // enter code here that interprets the mouse's
    // x, y position as H ans S (I suggest to set
    // V to 1.0) and converts that tripple to RGB
    return Vec3{x,y,1.0f};
  }
  
  
  virtual void init() {
    glEnv.setTitle("Color Picker");

    for (uint32_t y = 0;y<image.height;++y) {
      for (uint32_t x = 0;x<image.width;++x) {
        const Vec3 rgb = convertPosToHSV(float(x)/image.width, float(y)/image.height);
        image.setNormalizedValue(x,y,0,rgb.x());
        image.setNormalizedValue(x,y,1,rgb.y());
        image.setNormalizedValue(x,y,2,rgb.z());
        image.setValue(x,y,3,255);
      }
    }
  }
  
  virtual void mouseMove(double xPosition, double yPosition) {
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;

    const Vec3 hsv{float(360*xPosition/s.width),float(1.0-yPosition/s.height),1.0f};
    const Vec3 rgb = convertPosToHSV(float(xPosition/s.width), float(1.0-yPosition/s.height));
    std::cout << "HSV: " << hsv << " --> RGB: " << rgb << std::endl;
  }
    
  virtual void draw() {
    drawImage(image);
  }

} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}
