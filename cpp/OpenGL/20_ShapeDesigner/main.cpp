#include <iostream>
#include <GLApp.h>
#include <Vec2.h>
#include <Vec4.h>
#include <Mat4.h>

class MyGLApp : public GLApp {
public:
  MyGLApp() :
  GLApp(800,800,4, "Shape Designer", false, false)
  {}
  
  virtual void init() override {
    glEnv.setCursorMode(CursorMode::HIDDEN);
    GL(glClearColor(0,0,0,0));
  }
  
  virtual void mouseMove(double xPosition, double yPosition) override {
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
    mousePos = Vec2{float(xPosition/s.width),float(1.0-yPosition/s.height)};
  }
  
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override {
    if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_PRESS) {
      if (restartPrimitive) {
        restartPrimitive = false;
      } else {
        if (shape.size() > 0 && shape.size()%2 == 0) shape.push_back(shape[shape.size()-1]);
      }
      shape.push_back(Vec2{float(int(mousePos.x()*gridCells+0.5f)/gridCells) *2.0f-1.0f,
                           float(int(mousePos.y()*gridCells+0.5f)/gridCells) *2.0f-1.0f});
    }
  }

  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_ESCAPE:
          closeWindow();
          break;
        case GLFW_KEY_C:
          shape.clear();
          break;
        case GLFW_KEY_U:
          if (shape.size() > 0) {
            shape.pop_back();
            if (shape.size() > 0 && shape.size()%2 == 1) shape.pop_back();
          }
          undo = true;
          break;
        case GLFW_KEY_D:
          dumpShape();
          break;
        case GLFW_KEY_R:
          restartPrimitive = !restartPrimitive;
          break;
        case GLFW_KEY_RIGHT_BRACKET:   // US key for german +
          gridCells += 10.0;
          break;
        case GLFW_KEY_SLASH:           // US key for german -
          gridCells = std::max(gridCells - 10.0, 10.0);
          break;
      }
    }
  }
  
  void dumpShape() {
    std::cout << "{";
    for (size_t i = 0;i<shape.size();++i) {
      std::cout << "Vec2{"<< shape[i].x() <<  ", "  << shape[i].y();
      if (i < shape.size()-1 )
        std::cout << "}, ";
      else
        std::cout << "}";
    }
    std::cout << "}" << std::endl;
  }
  
  virtual void draw() override{
    GL(glClear(GL_COLOR_BUFFER_BIT));
    Vec2 gridMousePos{float(int(mousePos.x()*gridCells+0.5f)/gridCells),
                      float(int(mousePos.y()*gridCells+0.5f)/gridCells)};

    std::vector<float> glShape;
    float y = 0.0f;
    while (y <= 1.0) {
      float x = 0.0f;
      while (x <= 1.0) {
        
        glShape.push_back(x*2.0f-1.0f); glShape.push_back(y*2.0f-1.0f); glShape.push_back(0.0f);
        glShape.push_back(0.2f); glShape.push_back(0.2f); glShape.push_back(0.2f); glShape.push_back(1.0f);

        x += 1.0f/gridCells;
      }
      y += 1.0f/gridCells;
    }
    drawPoints(glShape,5);
    glShape.clear();
    
    glShape.push_back(mousePos.x()*2.0f-1.0f); glShape.push_back(mousePos.y()*2.0f-1.0f); glShape.push_back(0.0f);
    glShape.push_back(0.2f); glShape.push_back(0.2f); glShape.push_back(0.2f); glShape.push_back(1.0f);
    glShape.push_back(gridMousePos.x() *2.0f-1.0f); glShape.push_back(gridMousePos.y() *2.0f-1.0f); glShape.push_back(0.0f);
    glShape.push_back(1.0f); glShape.push_back(1.0f); glShape.push_back(1.0f);  glShape.push_back(1.0f);
    drawPoints(glShape,10);
    
    glShape.clear();
    for (size_t i = 0;i<(shape.size()/2)*2;++i) {
      glShape.push_back(shape[i].x());
      glShape.push_back(shape[i].y());
      glShape.push_back(0.0f);
      glShape.push_back(1.0f); glShape.push_back(1.0f); glShape.push_back(1.0f);  glShape.push_back(1.0f);
    }

    if (!restartPrimitive && shape.size()) {
      glShape.push_back(shape[shape.size()-1].x());
      glShape.push_back(shape[shape.size()-1].y());
      glShape.push_back(0.0f);
      glShape.push_back(0.2f); glShape.push_back(0.2f); glShape.push_back(0.2f);  glShape.push_back(1.0f);
      glShape.push_back(gridMousePos.x()*2.0f-1.0f); glShape.push_back(gridMousePos.y()*2.0f-1.0f); glShape.push_back(0.0f);
      glShape.push_back(0.2f); glShape.push_back(0.2f); glShape.push_back(0.2f); glShape.push_back(1.0f);
    }
    drawLines(glShape, LineDrawType::LIST);
  }

private:
  bool undo{true};
  bool restartPrimitive{false};
  double gridCells{100.0};
  Vec2 mousePos;
  std::vector<Vec2> shape;
    
} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}
