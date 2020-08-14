#pragma once

#include <vector>

#include "GLEnv.h"  

#include "GLTexture2D.h"
#include "GLDepthBuffer.h"

class GLFramebuffer {
public:
    GLFramebuffer();
    ~GLFramebuffer();

    const GLint getId() const;

    void bind(const GLTexture2D& t, const GLDepthBuffer& d);
    void bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLDepthBuffer& d);
    void bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLTexture2D& t2, const GLDepthBuffer& d);
    void bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLTexture2D& t2, const GLTexture2D& t3, const GLDepthBuffer& d);

    void bind(const GLTexture2D& t);
    void bind(const GLTexture2D& t0, const GLTexture2D& t1);
    void bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLTexture2D& t2);
    void bind(const GLTexture2D& t0, const GLTexture2D& t1, const GLTexture2D& t2, const GLTexture2D& t3);

    void unbind();

    bool checkBinding() const;
    
private:
	GLuint id;
};
