#pragma once

#include <vector>

#include <GL/glew.h>  
#include <GLFW/glfw3.h>

class GLBuffer {
public:
	GLBuffer(GLenum target);
	~GLBuffer();
	void setData(const std::vector<GLfloat>& data, size_t valuesPerElement,GLenum usage=GL_STATIC_DRAW);
	void setData(const std::vector<GLuint>& data);
	void connectVertexAttrib(GLint location, size_t elemCount, size_t offset=0) const;
	void bind() const;
	
private:
	GLenum target;
	GLuint bufferID;
	size_t elemSize;
	size_t stride;
	GLenum type;
};