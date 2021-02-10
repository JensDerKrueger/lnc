#include <array>
#include <cmath>

#include <GLApp.h>
#include <bmp.h>

#include "Flowfield.h"

class MyGLApp : public GLApp {
public:
  const uint32_t fieldSize{256};
  //Flowfield flow = Flowfield::genDemo(fieldSize, DemoType::SATTLE);
  
  Flowfield flow = Flowfield::fromFile("four_sector_128.txt");
  Image inputImage = BMP::load("noise.bmp");
  Image licImage{fieldSize,fieldSize,3};
  
  virtual void init() override {
    glEnv.setTitle("LIC demo");
    GL(glDisable(GL_CULL_FACE));
    GL(glDisable(GL_DEPTH_TEST));
    GL(glClearColor(0,0,0,0));
    computeLIC();
  }
  
  std::vector<Vec2> computeCurve(float x, float y, float z, size_t steps) {
    std::vector<Vec2> r;
    
    Vec2 pos{x,y};
    r.push_back(pos);
    
    const Vec2 delta{0.5f/flow.getSizeX(),0.5f/flow.getSizeY()};
    
    for (size_t i = 0;i<steps;++i) {
      const Vec3 v = flow.interpolate(Vec3(pos.x(), pos.y(), z));
      pos = pos + Vec2::normalize(Vec2{v.x(),v.y()}) * delta;
      if (pos.x() < 0.0 || pos.x() > 1.0 || pos.y() < 0.0 || pos.y() > 1.0) break;
      r.push_back(pos);
    }

    pos = Vec2{x,y};
    for (size_t i = 0;i<steps;++i) {
      const Vec3 v = flow.interpolate(Vec3(pos.x(), pos.y(), z));
      pos = pos + Vec2::normalize(Vec2{v.x(),v.y()}) * delta;
      if (pos.x() < 0.0 || pos.x() > 1.0 || pos.y() < 0.0 || pos.y() > 1.0) break;
      r.push_back(pos);
    }

    return r;
  }

  void licStep(Image& image, size_t steps) {
    for (size_t y = 0; y < image.height;++y) {
      for (size_t x = 0; x < image.width;++x) {
        std::vector<Vec2> trace = computeCurve(float(x)/image.width,
                                               float(y)/image.height,
                                               0.5f,
                                               steps);
        float value=0.0f;
        for (size_t i = 0;i<trace.size();++i) {
          const size_t u = size_t(trace[i].x() * inputImage.width + 0.5f);
          const size_t v = size_t(trace[i].y() * inputImage.height + 0.5f);
          value += inputImage.getValue(u,v,0);
        }
        
        const uint8_t output = uint8_t(value/trace.size());
        image.setValue(x,y,0,output);
        image.setValue(x,y,1,output);
        image.setValue(x,y,2,output);
      }
    }
  }

  void equalizeStep(Image& image) {
    const size_t pixelCount = image.data.size()/image.componentCount;
    std::array<size_t, 256> histogram;
    histogram.fill(0);
    
    for (size_t i = 0; i < pixelCount;++i) {
      histogram[image.data[i*image.componentCount]]++;
    }
    std::array<float, 256> cdf;
    cdf[0] = histogram[0];
    size_t cdf_min = cdf[0];
    for (size_t i = 1; i < 256;++i) {
      cdf[i]  = cdf[i-1] + histogram[i];
      if (cdf[i] != 0 && cdf_min == 0) {
        cdf_min = cdf[i];
      }
    }
    std::array<uint8_t, 256> remap;
    for (size_t i = 0; i < 256;++i) {
      remap[i] = uint8_t(round((cdf[i] - cdf_min)/(pixelCount-cdf_min) * 255) );
    }
    for (size_t i = 0;i<pixelCount;++i) {
      uint8_t remapped = remap[image.data[i*image.componentCount]];
      image.data[i*image.componentCount+0] = remapped;
      image.data[i*image.componentCount+1] = remapped;
      image.data[i*image.componentCount+2] = remapped;
    }
  }
  
  void computeLIC() {
    for (size_t i = 0;i<3;++i) {
      licStep(licImage,100);
      equalizeStep(licImage);
      inputImage = licImage;
    }
  }
  
  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT));
    drawImage(licImage);
  }
  
  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_ESCAPE:
          closeWindow();
          break;
          
        case GLFW_KEY_H:
          inputImage = licImage;
          computeLIC();
          break;
      }
    }
  }
} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}
