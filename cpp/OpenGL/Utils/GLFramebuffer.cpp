#include "GLFramebuffer.h"


GLFramebuffer::GLFramebuffer() :
	id(0)
{
    GL(glGenFramebuffers(1, &id));
}

GLFramebuffer::~GLFramebuffer() {
    GL(glDeleteFramebuffers(1, &id));
}

void GLFramebuffer::bind(const GLTexture2D& t, const GLDepthBuffer& d) {
    GL(glBindFramebuffer(GL_FRAMEBUFFER, id));
    GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, d.getId()));

    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t.getId(), 0));

    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    GL(glDrawBuffers(1, DrawBuffers));
}

void GLFramebuffer::bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLDepthBuffer& d) {
    GL(glBindFramebuffer(GL_FRAMEBUFFER, id));
    GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, d.getId()));

    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t0.getId(), 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, t1.getId(), 0));

    GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    GL(glDrawBuffers(2, DrawBuffers));
}

void GLFramebuffer::bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLTexture2D& t2, const GLDepthBuffer& d) {
    GL(glBindFramebuffer(GL_FRAMEBUFFER, id));
    GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, d.getId()));

    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t0.getId(), 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, t1.getId(), 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, t2.getId(), 0));

    GLenum DrawBuffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    GL(glDrawBuffers(3, DrawBuffers));
}

void GLFramebuffer::bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLTexture2D& t2, const GLTexture2D& t3, const GLDepthBuffer& d) {

    GL(glBindFramebuffer(GL_FRAMEBUFFER, id));
    GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, d.getId()));

    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t0.getId(), 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, t1.getId(), 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, t2.getId(), 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, t3.getId(), 0));

    GLenum DrawBuffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    GL(glDrawBuffers(4, DrawBuffers));
}

void GLFramebuffer::bind(const GLTexture2D& t) {
    GL(glBindFramebuffer(GL_FRAMEBUFFER, id));

    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t.getId(), 0));

    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    GL(glDrawBuffers(1, DrawBuffers));
}

void GLFramebuffer::bind(const GLTexture2D& t0, const GLTexture2D& t1) {
    GL(glBindFramebuffer(GL_FRAMEBUFFER, id));

    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t0.getId(), 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, t1.getId(), 0));

    GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    GL(glDrawBuffers(2, DrawBuffers));
}

void GLFramebuffer::bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLTexture2D& t2) {
    GL(glBindFramebuffer(GL_FRAMEBUFFER, id));

    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t0.getId(), 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, t1.getId(), 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, t2.getId(), 0));

    GLenum DrawBuffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    GL(glDrawBuffers(3, DrawBuffers));
}

void GLFramebuffer::bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLTexture2D& t2, const GLTexture2D& t3) {
    GL(glBindFramebuffer(GL_FRAMEBUFFER, id));

    GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t0.getId(), 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, t1.getId(), 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, t2.getId(), 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, t3.getId(), 0));

    GLenum DrawBuffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    GL(glDrawBuffers(4, DrawBuffers));
}

void GLFramebuffer::unbind() {
    GL(glDrawBuffer(GL_NONE));
    GL(glBindFramebuffer(GL_FRAMEBUFFER, id));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0));
    GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0));
    GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

bool GLFramebuffer::checkBinding() const {
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

const GLint GLFramebuffer::getId() const {
	return id;
}
