#include <sstream>
#include <iomanip>
typedef std::chrono::high_resolution_clock Clock;


#include "GLEnv.h"


void GLEnv::errorCallback(int error, const char* description) { 
	std::stringstream s;
	s << description << " " << error;
	throw GLException{s.str()};	
}

GLEnv::GLEnv(uint32_t w, uint32_t h, uint32_t s, const std::string& title, bool fpsCounter, bool sync) :
	window(nullptr),
	title(title),
	fpsCounter(fpsCounter)
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
		
		auto now = Clock::now();
		
		double fps = 1.0/((std::chrono::duration_cast<std::chrono::microseconds>(now - last).count())/1000000.0);
		last = now;		

		std::stringstream s;
		s << title << " (" << uint64_t(fps+0.5) << " fps)";		
		glfwSetWindowTitle(window, s.str().c_str());
		
	}
	
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