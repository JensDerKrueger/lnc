#pragma once

#include <string>
#include <exception>

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
		GLEnv(uint32_t w, uint32_t h, uint32_t s, const std::string& title);
		~GLEnv();
		
		void setKeyCallback(GLFWkeyfun keyCallback) const;
		void endOfFrame() const;
		bool shouldClose() const;
		
		Dimensions getFramebufferSize() const;
			
	private:
		GLFWwindow* window;
		
		static void errorCallback(int error, const char* description);
};