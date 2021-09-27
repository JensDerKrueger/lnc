#pragma once

#include <vector>

#include "GLEnv.h"

class GLDepthTexture {
public:
  GLDepthTexture(GLint magFilter=GL_LINEAR, GLint minFilter=GL_LINEAR,
                 GLint wrapX=GL_CLAMP_TO_EDGE, GLint wrapY=GL_CLAMP_TO_EDGE) {
    GL(glGenTextures(1, &id));
    GL(glBindTexture(GL_TEXTURE_2D, id));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapX));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapY));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS));
  }
  
  ~GLDepthTexture() {
    GL(glDeleteTextures(1, &id));
  }
      
  const GLuint getId() const {return id;}
  
  void setEmpty(uint32_t width, uint32_t height,
                GLDepthDataType dataType=GLDepthDataType::DEPTH24) {
    this->width = width;
    this->height = height;

    GL(glBindTexture(GL_TEXTURE_2D, id));
    GL(glPixelStorei(GL_PACK_ALIGNMENT ,1));
    GL(glPixelStorei(GL_UNPACK_ALIGNMENT ,1));

    switch (dataType) {
      case GLDepthDataType::DEPTH16:
        GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, GLsizei(width),
                        GLsizei(height), 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0));
        break;
      case GLDepthDataType::DEPTH24:
        GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, GLsizei(width),
                        GLsizei(height), 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0));
        break;
      case GLDepthDataType::DEPTH32:
        GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, GLsizei(width),
                        GLsizei(height), 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0));
        break;
    }
  }
  
  uint32_t getHeight() const {return height;}
  uint32_t getWidth() const {return width;}

  GLDepthDataType getType() const {return dataType;}
    
  void setFilter(GLint magFilter, GLint minFilter) {
    GL(glBindTexture(GL_TEXTURE_2D, id));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter));
  }
  
private:
  GLuint id;
  uint32_t width;
  uint32_t height;
  GLDepthDataType dataType;

};
