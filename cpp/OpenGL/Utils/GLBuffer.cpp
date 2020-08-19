#include <sstream>

#include "GLBuffer.h"
#include "GLEnv.h"


GLBuffer::GLBuffer(GLenum target) :
	target(target),
	bufferID(0),
	elemSize(0),
	stride(0),
	type(0)
{
	GL(glGenBuffers(1, &bufferID));
}

GLBuffer::~GLBuffer()  {
	GL(glBindBuffer(target, 0));
	GL(glDeleteBuffers(1, &bufferID));
}

void GLBuffer::setData(const std::vector<float>& data, size_t valuesPerElement, GLenum usage) {
	elemSize = sizeof(data[0]);
	stride = valuesPerElement*elemSize;
	type = GL_FLOAT;
	GL(glBindBuffer(target, bufferID));
	GL(glBufferData(target, elemSize*data.size(), data.data(), usage));
}

void GLBuffer::setData(const std::vector<GLuint>& data) {
	elemSize = sizeof(data[0]);
	stride = 1*elemSize;
	type = GL_UNSIGNED_INT;
	GL(glBindBuffer(target, bufferID));
	GL(glBufferData(target, elemSize*data.size(), data.data(), GL_STATIC_DRAW));
}

void GLBuffer::connectVertexAttrib(GLint location, size_t elemCount, size_t offset) const {
    if (type == 0) {
        throw GLException{"Need to call setData before connectVertexAttrib"};
    }
    
	GL(glBindBuffer(target, bufferID));
	GL(glEnableVertexAttribArray(location));
	GL(glVertexAttribPointer(location, GLsizei(elemCount), type, GL_FALSE, GLsizei(stride), (void*)(offset*elemSize)));
}

void GLBuffer::bind() const {
	GL(glBindBuffer(target, bufferID));
}
