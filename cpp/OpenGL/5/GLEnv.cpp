#include <sstream>

#include "GLEnv.h"


void GLEnv::errorCallback(int error, const char* description) { 
	std::stringstream s;
	s << description << " " << error;
	throw GLException{s.str()};	
}

GLEnv::GLEnv(uint32_t w, uint32_t h, uint32_t s, const std::string& title) :
	window(nullptr)
{
	glfwSetErrorCallback(errorCallback);  
	if (!glfwInit())
		throw GLException{"GLFW Init Failed"};

	glfwWindowHint(GLFW_SAMPLES, s);

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
}

GLEnv::~GLEnv() {
	glfwDestroyWindow(window);  
	glfwTerminate();	
}

void GLEnv::endOfFrame() {
	glfwSwapBuffers(window);
	glfwPollEvents();  
}

void GLEnv::setKeyCallback(GLFWkeyfun f) {
	glfwSetKeyCallback(window, f);
}

Dimensions GLEnv::getFramebufferSize() const {	
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	return Dimensions{uint32_t(width), uint32_t(height)};
}

bool GLEnv::shouldClose() const {
	return glfwWindowShouldClose(window);
}