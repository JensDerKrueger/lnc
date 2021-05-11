#include <GLApp.h>
#include <Grid2D.h>

class GLIPApp : public GLApp {
public:
  std::shared_ptr<Grid2D> last = std::make_shared<Grid2D>(512,512);
  std::shared_ptr<Grid2D> current = std::make_shared<Grid2D>(512,512);
  std::shared_ptr<Grid2D> next = std::make_shared<Grid2D>(512,512);
  Image image{512,512,3};
  
  const float c = 2.0f;
  const float dx = 0.5f;
  const float dt = 0.05f;
  const float alpha = (c*c*dt*dt) / (dx*dx);
  const float beta  = 2.0f - 4.0f*alpha;
    
  GLIPApp() : GLApp(512, 512, 4, "Water Surface Simulation")
  {
  }
     
  virtual void animate(double animationTime) override {

    const size_t w   = current->getWidth();
    const size_t h   = current->getHeight();
    
    for (size_t y = 0;y<current->getHeight();++y) {
      for (size_t x = 0;x<current->getWidth();++x) {
        
        const float left   = current->getValue((x+1)%w,y);
        const float right  = current->getValue((x+w-1)%w,y);
        const float top    = current->getValue(x,(y+1)%h);
        const float bottom = current->getValue(x,(y+h-1)%h);
        const float center = current->getValue(x,y);
        const float past   = last->getValue(x,y);

        const float v = alpha * (left+right+top+bottom) + beta * center - past;
        next->setValue(x,y,v);
        
        image.setValue(uint32_t(x),uint32_t(y),0, v > 0 ? uint8_t(v*500) : 0);
        image.setValue(uint32_t(x),uint32_t(y),1, v < 0 ? uint8_t(-v*500) : 0);
      }
    }
    
    std::shared_ptr<Grid2D> t = last;
    last    = current;
    current = next;
    next    = t;
  }

  virtual void draw() override {
    drawImage(image);
  }
  
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override {
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
    if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_PRESS) {
      current->setValue(uint32_t(current->getWidth()*float(xPosition/s.width)),
                        uint32_t(current->getHeight()*float(1.0-yPosition/s.height)), 1.0f);
    }

  }
  
  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_ESCAPE :
          closeWindow();
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
