#pragma once

#include <vector>

#include "GLEnv.h"

#include "GLTexture2D.h"
#include "GLTexture3D.h"
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

  void bind(const GLTexture3D& t, size_t slice, const GLDepthBuffer& d);
  void bind(const GLTexture3D& t0, size_t slice0, const GLTexture3D& t1, size_t slice1, const GLDepthBuffer& d);
  void bind(const GLTexture3D& t0, size_t slice0, const GLTexture3D& t1, size_t slice1, const GLTexture3D& t2, size_t slice2,
            const GLDepthBuffer& d);
  void bind(const GLTexture3D& t0, size_t slice0, const GLTexture3D& t1, size_t slice1, const GLTexture3D& t2, size_t slice2,
            const GLTexture3D& t3, size_t slice3, const GLDepthBuffer& d);

  void bind(const GLTexture3D& t, size_t slice);
  void bind(const GLTexture3D& t0, size_t slice0, const GLTexture3D& t1, size_t slice1);
  void bind(const GLTexture3D& t0, size_t slice0, const GLTexture3D& t1, size_t slice1, const GLTexture3D& t2, size_t slice2);
  void bind(const GLTexture3D& t0, size_t slice0, const GLTexture3D& t1, size_t slice1, const GLTexture3D& t2, size_t slice2,
            const GLTexture3D& t3, size_t slice3);

  void unbind2D();
  void unbind3D();
  bool checkBinding() const;
    
private:
  GLuint id;
  
  void setBuffers(size_t count, size_t width, size_t height);
};


// glFramebufferTexture3D
