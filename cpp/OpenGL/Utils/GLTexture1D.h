#pragma once

#include <vector>

#include "GLEnv.h"  

class GLTexture1D {
public:
	GLTexture1D(GLint magFilter=GL_NEAREST, GLint minFilter=GL_NEAREST,
				GLint wrapX=GL_REPEAT);
    
  GLTexture1D(const GLTexture1D& other);    
  GLTexture1D& operator=(GLTexture1D other);    
  
	~GLTexture1D();
	
	const GLint getId() const;	
	void setData(const std::vector<GLubyte>& data, uint32_t size, uint32_t componentCount=4);
		
private:
	GLuint id;
	GLenum internalformat;
	GLenum format;
	GLenum type;

  GLint magFilter;
  GLint minFilter;
  GLint wrapX;
  std::vector<GLubyte> data;
  uint32_t size;
  uint32_t componentCount;
};
