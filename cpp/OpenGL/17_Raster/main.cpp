#include <GLApp.h>
#include <Mat4.h>

class MyGLApp : public GLApp {
public:
  Image image{640,480};
  
  virtual void init() {
    glEnv.setTitle("Color Picker");

    for (uint32_t y = 0;y<image.height;++y) {
      for (uint32_t x = 0;x<image.width;++x) {
        const Vec3 rgb = Vec3::hsvToRgb({360*float(x)/image.width,float(y)/image.height,1.0f});
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
    const Vec3 rgb = Vec3::hsvToRgb(hsv);
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
