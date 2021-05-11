#include <iomanip>
#include <fstream>

#include <GLApp.h>
#include <bmp.h>
#include <Grid2D.h>

class GLIPApp : public GLApp {
public:
  Image image;
    
  GLIPApp() : GLApp(512, 512, 4, "Image Processing")
  {
  }
  
  void toGrayscale(bool uniform=false) {
    if (image.componentCount < 3) return;
    
    const Vec3 scale = uniform
                        ? Vec3{0.33333f,0.33333f,0.33333f}
                        : Vec3{0.299f,0.587f,0.114f};
        
    for (uint32_t y = 0;y<image.height;++y) {
      for (uint32_t x = 0;x<image.width;++x) {
        const uint8_t r = image.getValue(x,y,0);
        const uint8_t g = image.getValue(x,y,1);
        const uint8_t b = image.getValue(x,y,2);
        
        const uint8_t v = uint8_t(float(r) * scale.x() +
                                  float(g) * scale.y() +
                                  float(b) * scale.z());
        image.setValue(x,y,0,v);
        image.setValue(x,y,1,v);
        image.setValue(x,y,2,v);
      }
    }
  }

  void loadImage() {
    try {
      image = BMP::load("lenna.bmp");
    } catch (...) {
      image = Image(512,512);
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
  }
  
  virtual void init() override {
    loadImage();
  }
      
  virtual void draw() override {
    drawImage(image);
  }
  
  uint8_t getLumiValue(uint32_t x, uint32_t y) {
    switch (image.componentCount) {
      case 1 : return image.getValue(x,y,0);
      case 2 : return uint8_t(image.getValue(x,y,0)*0.5f + image.getValue(x,y,1)*0.5f);
      case 3 :
      case 4 : return uint8_t(image.getValue(x,y,0)*0.299f + image.getValue(x,y,1)*0.587f + image.getValue(x,y,2)*0.114f);
      default : return 0;
    }
  }
  
  std::string toString(bool bSmallTable=true) {
    const std::string lut1{"$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. "};
    const std::string lut2{"@%#*+=-:. "};
    const std::string& lut = bSmallTable ? lut2 : lut1;
    
    std::stringstream ss;
    for (uint32_t y = 0;y<image.height;y+=4) {
      for (uint32_t x = 0;x<image.width;x+=4) {
        const uint8_t v = getLumiValue(x,image.height-y);
        const char c = lut[(v*(lut.length()-1))/255];
        ss << c << c;
      }
      ss << "\n";
    }
    return ss.str();
  }
  
  void filter(const Grid2D& filter) {
    Image tempImage = image;
    
    const uint32_t hw = uint32_t(filter.getWidth()/2);
    const uint32_t hh = uint32_t(filter.getHeight()/2);
    
    for (uint32_t y = hh;y<image.height-hh;y+=1) {
      for (uint32_t x = hw;x<image.width-hw;x+=1) {
        for (uint32_t c = 0;c<image.componentCount;c+=1) {
          float conv = 0.0f;
          for (uint32_t u = 0;u<filter.getHeight();u+=1) {
            for (uint32_t v = 0;v<filter.getWidth();v+=1) {
              conv += float(image.getValue((x+u-hw),(y+v-hh),c)) * filter.getValue(u, v);
            }
          }
          tempImage.setValue(x,y,c,uint8_t(fabs(conv)));
        }
      }
    }
    
    image = tempImage;
  }
  
  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_ESCAPE :
          closeWindow();
          break;
        case GLFW_KEY_M :
          {
            Grid2D mean{3,3};
            mean.fill(1.0f/(mean.getHeight()*mean.getWidth()));
            filter(mean);
          }
          break;
        case GLFW_KEY_A :
          filter({3,3, {-1, 0, 1,
                        -2, 0, 2,
                        -1, 0, 1}});
          break;
        case GLFW_KEY_B:
          filter({3,3, {-1,-2,-1,
                         0, 0, 0,
                         1, 2, 1}});
        break;
        case GLFW_KEY_G :
          toGrayscale(false);
          break;
        case GLFW_KEY_H :
          toGrayscale(true);
          break;
        case GLFW_KEY_R :
          loadImage();
          break;
        case GLFW_KEY_T : {
          std::ofstream file{ "ascii.txt" };
          file << toString() << std::endl;
          break;
        }
      }
    }
  }
  
};

#ifdef _WIN32
#include <Windows.h>

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main(int argc, char** argv) {
#endif
  try {
    GLIPApp imageProcessing;
    imageProcessing.run();
  }
  catch (const GLException& e) {
    std::stringstream ss;
    ss << "Insufficient OpenGL Support " << e.what();
#ifndef _WIN32
    std::cerr << ss.str().c_str() << std::endl;
#else
    MessageBoxA(
      NULL,
      ss.str().c_str(),
      "OpenGL Error",
      MB_ICONERROR | MB_OK
    );
#endif
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
