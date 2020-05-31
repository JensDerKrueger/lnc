#include <sstream>
#include <iostream>

#include "GLEnv.h"

void GLEnv::errorCallback(int error, const char* description) {  
	std::cerr << "Fatal Error: " << description << " (" << error << ")" << std::endl;
}  

GLEnv::GLEnv(uint32_t w, uint32_t h, uint32_t s, const std::string& title) :
	window(nullptr)
{
	glfwSetErrorCallback(errorCallback);  
	if (!glfwInit()) {  
		throw GLException("Glfw init failed.");
	}  

	glfwWindowHint(GLFW_SAMPLES, s);

	window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
	if (window == nullptr) {
		glfwTerminate();
		throw GLException("Failed to open GLFW window.");
	}  

	glfwMakeContextCurrent(window);  

	GLenum err{glewInit()};
	if (err != GLEW_OK) {  
		std::stringstream s;
		s << "Failed to init GLEW " << glewGetErrorString(err); 
		glfwTerminate();  
		throw GLException(s.str());
	}
	
}

GLEnv::~GLEnv() {
	glfwTerminate();
}


void GLEnv::setKeyCallback(GLFWkeyfun keyCallback) const {
	glfwSetKeyCallback(window, keyCallback);
}

void GLEnv::endOfFrame() const{	
	glfwSwapBuffers(window);
	glfwPollEvents();
}

bool GLEnv::shouldClose() const {	
	return glfwWindowShouldClose(window);
}

Dimensions GLEnv::getFramebufferSize() const {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);	
	return {static_cast<uint32_t>(width),static_cast<uint32_t>(height)};
}