#pragma once

#include <exception>
#include <string>
#include <chrono>

#include <GL/glew.h>  
#include <GLFW/glfw3.h>

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
		GLEnv(uint32_t w, uint32_t h, uint32_t s, const std::string& title, bool fpsCounter=false, bool sync=true);
		~GLEnv();
		void setKeyCallback(GLFWkeyfun f);
		
		Dimensions getFramebufferSize() const;	
		bool shouldClose() const;
		void endOfFrame();
		
		void setFPSCounter(bool fpsCounter);
		void setSync(bool sync);
	private:
		GLFWwindow* window;
		std::string title;
		bool fpsCounter;
		std::chrono::high_resolution_clock::time_point last;
		
		static void errorCallback(int error, const char* description);		
};