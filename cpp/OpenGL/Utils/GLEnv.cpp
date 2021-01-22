#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <iostream>

typedef std::chrono::high_resolution_clock Clock;

#include "GLEnv.h"
#include "GLDebug.h"


void GLEnv::checkGLError(const std::string& id) {
  GLenum e = glGetError();
  if (e != GL_NO_ERROR) {
    std::cerr << "An openGL error occured:" << errorString(e) << " (" << e << ") at " << id << std::endl;
  }
}

void GLEnv::errorCallback(int error, const char* description) {
  // ignore known issue on Apple M1
  if (error == 65544) return;
  std::stringstream s;
  s << description << " (Error number: " << error << ")";
  throw GLException{s.str()};
}

GLEnv::GLEnv(uint32_t w, uint32_t h, uint32_t s, const std::string& title, bool fpsCounter, bool sync, int major, int minor, bool core) :
  window(nullptr),
  title(title),
  fpsCounter(fpsCounter),
  last(Clock::now())
{
  glfwSetErrorCallback(errorCallback);
  if (!glfwInit())
    throw GLException{"GLFW Init Failed"};

  glfwWindowHint(GLFW_SAMPLES, s);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
  
  if (core) {
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  }

  window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
  if (window == nullptr) {
    std::stringstream s;
    s << "Failed to open GLFW window.";
    glfwTerminate();
    throw GLException{s.str()};
  }

  glfwMakeContextCurrent(window);

  GLenum err{glewInit()};
  if (err != GLEW_OK) {
    std::stringstream s;
    s << "Failed to init GLEW " << glewGetErrorString(err) << std::endl;
    glfwTerminate();
    throw GLException{s.str()};
  }
  
  setSync(sync);
}

GLEnv::~GLEnv() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void GLEnv::setSync(bool sync) {
  if (sync)
    glfwSwapInterval( 1 );
  else
    glfwSwapInterval( 0 );
}


void GLEnv::setFPSCounter(bool fpsCounter) {
  this->fpsCounter = fpsCounter;
}

void GLEnv::endOfFrame() {
  glfwSwapBuffers(window);
  glfwPollEvents();
  
  if (fpsCounter) {
    frameCount++;
    auto now = Clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - last);
    if(diff.count() > 1e6) {
        auto fps = static_cast<double>(frameCount)/diff.count()*1.e6;
            std::stringstream s;
            s << title << " (" << static_cast<int>(std::ceil(fps)) << " fps)";
            glfwSetWindowTitle(window, s.str().c_str());
            frameCount = 0;
            last = now;
        }
  }
}

void GLEnv::setKeyCallback(GLFWkeyfun f) {
  glfwSetKeyCallback(window, f);
}

void GLEnv::setResizeCallback(GLFWframebuffersizefun f) {
  glfwSetFramebufferSizeCallback(window, f);
}

void GLEnv::setMouseCallbacks(GLFWcursorposfun p, GLFWmousebuttonfun b, GLFWscrollfun s) {
  glfwSetCursorPosCallback(window, p);
  glfwSetMouseButtonCallback(window, b);
  glfwSetScrollCallback(window, s);
}

Dimensions GLEnv::getFramebufferSize() const {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  return Dimensions{uint32_t(width), uint32_t(height)};
}


Dimensions GLEnv::getWindowSize() const {
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  return Dimensions{uint32_t(width), uint32_t(height)};
}

bool GLEnv::shouldClose() const {
  return glfwWindowShouldClose(window);
}

void GLEnv::setClose() {
  return glfwSetWindowShouldClose(window, GL_TRUE);
}

void GLEnv::setTitle(const std::string& title) {
  this->title = title;
}

void GLEnv::setCursorMode(CursorMode mode) {
  switch (mode) {
    case CursorMode::NORMAL :
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      break;
    case CursorMode::HIDDEN :
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
      break;
    case CursorMode::FIXED :
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      break;
  }
}

