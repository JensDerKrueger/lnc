#include "GLFramebuffer.h"


GLFramebuffer::GLFramebuffer() :
	id(0)
{
    glGenFramebuffers(1, &id);
}

GLFramebuffer::~GLFramebuffer() {
    glDeleteFramebuffers(1, &id);
}

void GLFramebuffer::bind(const GLTexture2D& t, const GLDepthBuffer& d) {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, d.getId());

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t.getId(), 0);

    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);
}

void GLFramebuffer::bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLDepthBuffer& d) {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, d.getId());

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t0.getId(), 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, t1.getId(), 0);

    GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, DrawBuffers);
}

void GLFramebuffer::bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLTexture2D& t2, const GLDepthBuffer& d) {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, d.getId());

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t0.getId(), 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, t1.getId(), 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, t2.getId(), 0);

    GLenum DrawBuffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, DrawBuffers);
}

void GLFramebuffer::bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLTexture2D& t2, const GLTexture2D& t3, const GLDepthBuffer& d) {

    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, d.getId());

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t0.getId(), 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, t1.getId(), 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, t2.getId(), 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, t3.getId(), 0);

    GLenum DrawBuffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, DrawBuffers);
}

void GLFramebuffer::bind(const GLTexture2D& t) {
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t.getId(), 0);

    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);
}

void GLFramebuffer::bind(const GLTexture2D& t0, const GLTexture2D& t1) {
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t0.getId(), 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, t1.getId(), 0);

    GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, DrawBuffers);
}

void GLFramebuffer::bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLTexture2D& t2) {
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t0.getId(), 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, t1.getId(), 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, t2.getId(), 0);

    GLenum DrawBuffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, DrawBuffers);
}

void GLFramebuffer::bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLTexture2D& t2, const GLTexture2D& t3) {
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t0.getId(), 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, t1.getId(), 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, t2.getId(), 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, t3.getId(), 0);

    GLenum DrawBuffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, DrawBuffers);
}

void GLFramebuffer::unbind() {
    glDrawBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool GLFramebuffer::checkBinding() const {
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

const GLint GLFramebuffer::getId() const {
	return id;
}
