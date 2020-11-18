#pragma once

#include <exception>
#include <string>
#include <chrono>

#include <GL/glew.h>  
#include <GLFW/glfw3.h>

#include "GLDebug.h"

enum class GLDataType {BYTE, HALF, FLOAT};
enum class CursorMode {NORMAL, HIDDEN, FIXED};

class GLException : public std::exception {
	public:
		GLException(const std::string& whatStr) : whatStr(whatStr) {}
		virtual const char* what() const throw() {
			return whatStr.c_str();
		}
	private:
		std::string whatStr;
};

struct Dimensions {
	uint32_t width;
	uint32_t height;
	
	float aspect() const {return float(width)/float(height);}
};

class GLEnv {
public:
  GLEnv(uint32_t w, uint32_t h, uint32_t s, const std::string& title, bool fpsCounter=false, bool sync=true, int major=2, int minor=1, bool core=false);
  ~GLEnv();
  void setKeyCallback(GLFWkeyfun f);
  void setMouseCallbacks(GLFWcursorposfun p, GLFWmousebuttonfun b, GLFWscrollfun s);
  void setResizeCallback(GLFWframebuffersizefun f);
  
  Dimensions getFramebufferSize() const;
  Dimensions getWindowSize() const;
  bool shouldClose() const;
  void setClose();
  void endOfFrame();
  
  void setCursorMode(CursorMode mode);
  
  void setFPSCounter(bool fpsCounter);
  void setSync(bool sync);

  static void checkGLError(const std::string& id);

  void setTitle(const std::string& title);

private:
  GLFWwindow* window;
  std::string title;
  bool fpsCounter;
  std::chrono::high_resolution_clock::time_point last;
  uint64_t frameCount;
		
  static void errorCallback(int error, const char* description);
};
