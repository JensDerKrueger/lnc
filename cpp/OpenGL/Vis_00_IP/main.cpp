#include <iomanip>

#include <GLApp.h>
#include <bmp.h>
#include <Grid2D.h>

class GLIPApp : public GLApp {
public:
  Image image;
    
  GLIPApp() : GLApp(512, 512, 4, "Image Processing")
  {
  }
  
  void toGraysascale(bool poor) {
    const Vec3 scale = poor
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
  
  virtual void init() override {
    image = BMP::load("lenna.bmp");
  }
      
  virtual void draw() override {
    drawImage(image);
  }

  std::string toString() {
    std::stringstream ss;
    for (uint32_t y = 0;y<image.height;y+=4) {
      for (uint32_t x = 0;x<image.width;x+=4) {
        const uint8_t v = image.getValue(x,image.height-y,0);
        
        if (v < 20)  ss << "  "; else
        if (v < 40)  ss << ".."; else
        if (v < 80)  ss << "::"; else
        if (v < 120) ss << ";;"; else
        if (v < 140) ss << "oo"; else
        if (v < 160) ss << "xx"; else
        if (v < 180) ss << "%%"; else
        if (v < 200) ss << "##"; else
        ss << "@@";
        
      }
      ss << "\n";
    }
    return ss.str();
  }
  
  void filter(const Grid2D filter) {
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
            mean.fill(1.0f/9.0f);
            filter(mean);
          }
          break;
        case GLFW_KEY_A :
          {
            Grid2D sobelH{3,3, {-1,0,1,
                                -2,0,2,
                                -1,0,1}};
            filter(sobelH);
          }
          break;
        case GLFW_KEY_B:
        {
          Grid2D sobelV{ 3,3, {-1,-2,-1,
                                0,0,0,
                                1,2, 1} };
          filter(sobelV);
        }
        break;
        case GLFW_KEY_G :
          toGraysascale(false);
          break;
        case GLFW_KEY_H :
          toGraysascale(true);
          break;
        case GLFW_KEY_R :
          image = BMP::load("lenna.bmp");
          break;
        case GLFW_KEY_T :
          std::cout << toString() << std::endl;
          break;
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