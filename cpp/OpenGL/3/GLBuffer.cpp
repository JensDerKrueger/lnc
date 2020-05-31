#include <sstream>

#include "GLBuffer.h"


GLBuffer::GLBuffer(GLenum target) :
	target(target),
	bufferID(0),
	elemSize(0),
	stride(0),
	type(0)
{
	glGenBuffers(1, &bufferID);	
}

GLBuffer::~GLBuffer()  {
	glBindBuffer(target, 0);
	glDeleteBuffers(1, &bufferID);
}

void GLBuffer::setData(const std::vector<float>& data, size_t valuesPerElement) {
	elemSize = sizeof(data[0]);
	stride = valuesPerElement*elemSize;
	type = GL_FLOAT;
	glBindBuffer(target, bufferID);
	glBufferData(target, elemSize*data.size(), data.data(), GL_STATIC_DRAW); 	
}

void GLBuffer::setData(const std::vector<GLuint>& data) {
	elemSize = sizeof(data[0]);
	stride = 1*elemSize;
	type = GL_UNSIGNED_INT;
	glBindBuffer(target, bufferID);
	glBufferData(target, elemSize*data.size(), data.data(), GL_STATIC_DRAW); 	
}

void GLBuffer::connectVertexAttrib(GLint location, size_t count, size_t offset) const {
	glBindBuffer(target, bufferID);
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, count, type, GL_FALSE, stride, (void*)(offset*elemSize));
}

void GLBuffer::bind() const {
	glBindBuffer(target, bufferID);
}