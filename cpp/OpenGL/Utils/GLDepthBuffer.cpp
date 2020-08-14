#include "GLDepthBuffer.h"


GLDepthBuffer::GLDepthBuffer(uint32_t width, uint32_t height) :
	id(0),
    width(width),
    height(height)
{
    GL(glGenRenderbuffers(1, &id));
    GL(glBindRenderbuffer(GL_RENDERBUFFER, id));
    GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height));
}

GLDepthBuffer::~GLDepthBuffer() {
	GL(glDeleteRenderbuffers(1, &id));
}

const GLint GLDepthBuffer::getId() const {
	return id;
}
