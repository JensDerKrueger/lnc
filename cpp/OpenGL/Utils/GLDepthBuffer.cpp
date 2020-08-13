#include "GLDepthBuffer.h"


GLDepthBuffer::GLDepthBuffer(uint32_t width, uint32_t height) :
	id(0),
    width(width),
    height(height)
{
    glGenRenderbuffers(1, &id);
    glBindRenderbuffer(GL_RENDERBUFFER, id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
}

GLDepthBuffer::~GLDepthBuffer() {
	glDeleteRenderbuffers(1, &id);
}

const GLint GLDepthBuffer::getId() const {
	return id;
}
