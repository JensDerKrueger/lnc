#pragma once

#include <vector>
#include <string>
#include <exception>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Mat4.h"
#include "GLTexture1D.h"
#include "GLTexture2D.h"
#include "GLTexture3D.h"

class ProgramException : public std::exception {
	public:
		ProgramException(const std::string& whatStr) : whatStr(whatStr) {}
		virtual const char* what() const throw() {
			return whatStr.c_str();
		}
	private:
		std::string whatStr;
};

class GLProgram {
public:
	~GLProgram();
	
	static GLProgram createFromFiles(const std::vector<std::string>& vs, const std::vector<std::string>& fs,
                                   const std::vector<std::string>& gs = std::vector<std::string>());
	static GLProgram createFromStrings(const std::vector<std::string>& vs, const std::vector<std::string>& fs,
                                     const std::vector<std::string>& gs = std::vector<std::string>());

	static GLProgram createFromFile(const std::string& vs, const std::string& fs, const std::string& gs="");
	static GLProgram createFromString(const std::string& vs, const std::string& fs, const std::string& gs="");
	
  GLProgram(const GLProgram& other);
  GLProgram& operator=(const GLProgram& other);
  
	GLint getAttributeLocation(const std::string& id) const;
	GLint getUniformLocation(const std::string& id) const;

  void setUniform(const std::string& id, float value) const;
  void setUniform(const std::string& id, const Vec2& value) const;
  void setUniform(const std::string& id, const Vec3& value) const;
  void setUniform(const std::string& id, const Vec4& value) const;
  void setUniform(const std::string& id, int value) const;
  void setUniform(const std::string& id, const Vec2i& value) const;
  void setUniform(const std::string& id, const Mat4& value, bool transpose=false) const;
  
  void setTexture(const std::string& id, const GLTexture1D& texture, GLuint unit=0) const;
  void setTexture(const std::string& id, const GLTexture2D& texture, GLuint unit=0) const;
  void setTexture(const std::string& id, const GLTexture3D& texture, GLuint unit=0) const;

	void setUniform(GLint id, float value) const;
  void setUniform(GLint id, const Vec2& value) const;
	void setUniform(GLint id, const Vec3& value) const;
  void setUniform(GLint id, const Vec4& value) const;
  void setUniform(GLint id, int value) const;
  void setUniform(GLint id, const Vec2i& value) const;
	void setUniform(GLint id, const Mat4& value, bool transpose=false) const;
	
  void setTexture(GLint id, const GLTexture1D& texture, GLuint unit=0) const;
  void setTexture(GLint id, const GLTexture2D& texture, GLuint unit=0) const;
	void setTexture(GLint id, const GLTexture3D& texture, GLuint unit=0) const;
	
  void unsetTexture1D(GLuint unit) const;
  void unsetTexture2D(GLuint unit) const;
  void unsetTexture3D(GLuint unit) const;

	void enable() const;
	void disable() const;

private:
	GLuint glVertexShader;
	GLuint glFragmentShader;
	GLuint glGeometryShader;
	GLuint glProgram;
  
  std::vector<std::string> vertexShaderStrings;
  std::vector<std::string> fragmentShaderStrings;
  std::vector<std::string> geometryShaderStrings;
	
	static std::string loadFile(const std::string& filename);
	
	static GLuint createShader(GLenum type, const GLchar** src, GLsizei count);
	static void checkAndThrow();
	static void checkAndThrowShader(GLuint shader);
	static void checkAndThrowProgram(GLuint program);

  GLProgram(std::vector<std::string> vertexShaderStrings, std::vector<std::string> fragmentShaderStrings, std::vector<std::string> geometryShaderStrings);

  void programFromVectors(std::vector<std::string> vs, std::vector<std::string> fs, std::vector<std::string> gs);
};
