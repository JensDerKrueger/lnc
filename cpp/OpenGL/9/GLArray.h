#pragma once

#include <vector>

#include <GL/glew.h>  
#include <GLFW/glfw3.h>

#include "GLBuffer.h"
#include "GLProgram.h"

class GLArray {
public:
	GLArray();
	~GLArray();
	
	void bind() const;
	void connectVertexAttrib(const GLBuffer& buffer, const GLProgram& program, const std::string& variable, size_t elemCount, size_t offset=0) const;
	void connectIndexBuffer(const GLBuffer& buffer) const;
	
private:
	GLuint glId;
	
};