#include <limits>
#include <algorithm>
#include <cmath>

#include <GLApp.h>

class SignedDistanceTransform : public GLApp {
public:
  Image distanceImage{500,500,4};
  std::vector<bool> I;
  std::vector<float> d;
  std::vector<Vec2ui> p;
  
  const float d1{1.0f};
  const float d2{1.4142135624f};
  
  const float INV = std::numeric_limits<float>::max();
  const Vec2ui NO_POS{std::numeric_limits<uint32_t>::max(),
                      std::numeric_limits<uint32_t>::max()};
  
  Vec2ui imagePos;
  
  SignedDistanceTransform() :
    GLApp(500,500,1)
  {
    const size_t s = distanceImage.width*distanceImage.height;
    I.resize(s);
    d.resize(s);
    p.resize(s);
  }
  
  virtual void init() override {
    glEnv.setTitle("Signed Distance Transform");
  }
  
  virtual void mouseMove(double xPosition, double yPosition) override{
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
    imagePos = Vec2ui{uint32_t(float(xPosition)/s.width*distanceImage.width),
                      uint32_t((1.0f-float(yPosition)/s.height)*distanceImage.height)};
    computeDRA(false);
    updateImage();
  }
    
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      if (state == GLFW_PRESS) {
        computeDRA(true);
        updateImage();
      }
    }
  }

  virtual void keyboard(int key, int scancode, int action, int mods) override{
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_ESCAPE :
          closeWindow();
          break;
        case GLFW_KEY_C :
          std::fill(I.begin(), I.end(), false);
          computeDRA(false);
          updateImage();
          break;
      }
    }
  }
  
  size_t index(size_t x, size_t y) {
    return x + y * distanceImage.width;
  }
  
  float dist(size_t x, size_t y) {
    const size_t i = index(x,y);
    return sqrtf( (x-p[i].x())*(x-p[i].x()) + (y-p[i].y())*(y-p[i].y()) );
  }
  
  void computeDRA(bool permanent) {
    
    const bool last = I[index(imagePos.x(), imagePos.y())];
    I[index(imagePos.x(), imagePos.y())] = true;
        
    for (size_t i = 0;i<I.size();++i) {
      d[i] = INV;
      p[i] = NO_POS;
    }
    
    for (size_t y = 1; y<distanceImage.height-1; y++ ) {
      for (size_t x = 1; x<distanceImage.width-1; x++ ) {
        if (I[index(x-1,y)] != I[index(x,y)] ||
            I[index(x+1,y)] != I[index(x,y)] ||
            I[index(x,y+1)] != I[index(x,y)] ||
            I[index(x,y-1)] != I[index(x,y)]) {
          d[index(x,y)] = 0;
          p[index(x,y)] = Vec2ui(uint32_t(x),uint32_t(y));
        }
      }
    }
   
    for (size_t y = 1; y<distanceImage.height-1; y++ ) {
      for (size_t x = 1; x<distanceImage.width-1; x++ ) {
        if (d[index(x-1,y-1)]+d2 < d[index(x,y)]) {
          p[index(x,y)] = p[index(x-1,y-1)];
          d[index(x,y)] = dist(x, y);
        }
        if (d[index(x,y-1)]+d1 < d[index(x,y)]) {
          p[index(x,y)] = p[index(x,y-1)];
          d[index(x,y)] = dist(x, y);
        }
        if (d[index(x+1,y-1)]+d2 < d[index(x,y)]) {
          p[index(x,y)] = p[index(x+1,y-1)];
          d[index(x,y)] = dist(x, y);
        }
        if (d[index(x-1,y)]+d1 < d[index(x,y)]) {
          p[index(x,y)] = p[index(x-1,y)];
          d[index(x,y)] = dist(x, y);
        }
      }
    }
    
    for (size_t y = distanceImage.height-2; y>=1; y-- ) {
      for (size_t x = distanceImage.width-2; x>=1; x--) {
        if (d[index(x+1,y)]+d1 < d[index(x,y)]) {
          p[index(x,y)] = p[index(x+1,y)];
          d[index(x,y)] = dist(x, y);
        }
        if (d[index(x-1,y+1)]+d2 < d[index(x,y)]) {
          p[index(x,y)] = p[index(x-1,y+1)];
          d[index(x,y)] = dist(x, y);
        }
        if (d[index(x,y+1)]+d1 < d[index(x,y)]) {
          p[index(x,y)] = p[index(x,y+1)];
          d[index(x,y)] = dist(x, y);
        }
        if (d[index(x+1,y+1)]+d2 < d[index(x,y)]) {
          p[index(x,y)] = p[index(x+1,y+1)];
          d[index(x,y)] = dist(x, y);
        }
      }
    }
    
    for (size_t i = 0;i<I.size();++i) {
      if (!I[i]) d[i] = -d[i];
    }
    
    if (!permanent) I[index(imagePos.x(), imagePos.y())] = last;
  }
  
  void updateImage() {
    float maxValue = 0.0f;
    for (size_t y = 0; y<distanceImage.height; y++ ) {
      for (size_t x = 0; x<distanceImage.width; x++ ) {
        const float current = d[index(x,y)];
        if (fabs(current) != INV && maxValue < fabs(current))
          maxValue = fabs(current);
      }
    }

    for (size_t y = 0; y<distanceImage.height; y++ ) {
      for (size_t x = 0; x<distanceImage.width; x++ ) {
        const float current = d[index(x,y)]/maxValue;
        
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
