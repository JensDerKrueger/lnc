#pragma once

#include <vector>

#include "GLEnv.h"  

class GLDepthBuffer {
public:
	GLDepthBuffer(uint32_t width, uint32_t height);
	~GLDepthBuffer();
	
	const GLint getId() const;
		
    uint32_t getWidth() const {return width;}
    uint32_t getHeight() const {return height;}
    
private:
	GLuint id;
    uint32_t width;
    uint32_t height;
};
