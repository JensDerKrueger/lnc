#pragma once

#include <vector>

#include "GLEnv.h"  

class GLTexture2D {
public:
	GLTexture2D(GLint magFilter=GL_NEAREST, GLint minFilter=GL_NEAREST,
				GLint wrapX=GL_REPEAT, GLint wrapY=GL_REPEAT);
	~GLTexture2D();
	
	const GLint getId() const;	
	void setData(const std::vector<GLubyte>& data, uint32_t width, uint32_t height, uint32_t componentCount=4);
		
private:
	GLuint id;
	GLenum internalformat;
	GLenum format;
	GLenum type;

};