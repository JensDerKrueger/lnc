#pragma once

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GLEnv.h"
#include "GLProgram.h"
#include "GLArray.h"
#include "GLBuffer.h"


class GLApp {
public:
  GLApp(uint32_t w=640, uint32_t h=480, uint32_t s=4,
        const std::string& title = "My OpenGL App",
        bool fpsCounter=true, bool sync=true, int major=4,
        int minor=1, bool core=true);
  
  void run();
  void setAnimation(bool animationActive) {
    this->animationActive = animationActive;
  }
  bool getAnimation() const {
    return animationActive;
  }

  virtual void init() {}
  virtual void draw() {}
  virtual void animate() {}
  
  virtual void resize(int width, int height) {}
  virtual void keyboard(int key, int scancode, int action, int mods) {}
  virtual void mouseMove(double xPosition, double yPosition) {}
  virtual void mouseButton(int button, int state, int mods) {}
  virtual void mouseWheel(double x_offset, double y_offset) {}
  
protected:
  GLEnv glEnv;
  GLProgram simpleProg;
  GLArray simpleArray;
  GLBuffer simpleVb;

  
private:
  bool animationActive;
  
  static GLApp* staticAppPtr;
  static void sizeCallback(GLFWwindow* window, int width, int height) {
    if (staticAppPtr) staticAppPtr->resize(width, height);
  }
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (staticAppPtr) staticAppPtr->keyboard(key, scancode, action, mods);
  }
  static void cursorPositionCallback(GLFWwindow* window, double xPosition, double yPosition) {
    if (staticAppPtr) staticAppPtr->mouseMove(xPosition, yPosition);
  }
  static void mouseButtonCallback(GLFWwindow* window, int button, int state, int mods) {
    if (staticAppPtr) staticAppPtr->mouseButton(button, state, mods);
  }
  static void scrollCallback(GLFWwindow* window, double x_offset, double y_offset) {
    if (staticAppPtr) staticAppPtr->mouseWheel(x_offset, y_offset);
  }
};
