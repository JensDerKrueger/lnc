#include <limits>
#include <algorithm>
#include <fstream>

#include <GLApp.h>
#include <Grid2D.h>
#include <bmp.h>

const uint32_t w{500};
const uint32_t h{500};

class SignedDistanceTransform : public GLApp {
public:
  Image distanceImage{w,h,4};
  Grid2D data{w,h};
  const float INV = std::numeric_limits<float>::max();
  
  Vec2ui imagePos;
  
  SignedDistanceTransform() :
    GLApp(w,h,1)
  {
    std::ifstream is{"data.bin", std::ios::binary};
    if (is.is_open()) {
      data = Grid2D(is);
      updateImage();
    }
    is.close();
  }
  
  virtual ~SignedDistanceTransform() {
    std::ofstream os{"data.bin", std::ios::binary};
    data.save(os);
    os.close();
  }
  
  virtual void init() override {
    glEnv.setTitle("Signed Distance Transform");
  }
  
  virtual void mouseMove(double xPosition, double yPosition) override{
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
    imagePos = Vec2ui{uint32_t(float(xPosition)/s.width*distanceImage.width),
                      uint32_t((1.0f-float(yPosition)/s.height)*distanceImage.height)};
    const float v = data.getValue(imagePos.x, imagePos.y);
    data.setValue(imagePos.x, imagePos.y, 1.0f);
    updateImage();
    data.setValue(imagePos.x, imagePos.y, v);
  }
    
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      if (state == GLFW_PRESS) {
        data.setValue(imagePos.x, imagePos.y, 1.0f);
        updateImage();
      }
    }
  }

  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_ESCAPE :
          closeWindow();
          break;
        case GLFW_KEY_I :
          data = Grid2D::fromBMP("helvetica_neue.bmp");
          updateImage();
          break;
        case GLFW_KEY_C :
          data.fill(0);
          updateImage();
          break;
      }
    }
  }
  
  void updateImage() {
    if (data.getWidth() != distanceImage.width ||
        data.getHeight() != distanceImage.height) {
      distanceImage = Image(uint32_t(data.getWidth()),uint32_t(data.getHeight()));
    }
    
    const float INV = std::numeric_limits<float>::max();
    const Grid2D distData = data.toSignedDistance(0.5f);
    
    float maxValue = 0.0f;
    for (size_t y = 0; y<distanceImage.height; y++ ) {
      for (size_t x = 0; x<distanceImage.width; x++ ) {
        const float current = distData.getValue(x, y);
        if (fabs(current) != INV && maxValue < fabs(current))
          maxValue = fabs(current);
      }
    }

    for (size_t y = 0; y<distanceImage.height; y++ ) {
      for (size_t x = 0; x<distanceImage.width; x++ ) {
        const float current = sinf(1.5707963268f*distData.getValue(x, y)/maxValue);
        
        const uint8_t r = current < 0  ? uint8_t(-current*255) : 0;
        const uint8_t g = current >= 0 ? uint8_t(current*255) : 0;

        distanceImage.setValue(uint32_t(x),uint32_t(y),0,g);
        distanceImage.setValue(uint32_t(x),uint32_t(y),1,r);
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
