#pragma once

#include <vector>
#include <string>
#include <exception>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Vec3.h"
#include "Mat4.h"

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
	
	static GLProgram createFromFiles(const std::vector<std::string>& vs, const std::vector<std::string>& fs);
	static GLProgram createFromStrings(const std::vector<std::string>& vs, const std::vector<std::string>& fs);

	static GLProgram createFromFiles(const std::string& vs, const std::string& fs);
	static GLProgram createFromStrings(const std::string& vs, const std::string& fs);
	
	GLint getAttributeLocation(const std::string& id) const;
	GLint getUniformLocation(const std::string& id) const;
			
	void setUniform(GLint id, float value) const;
	void setUniform(GLint id, const Vec3& value) const;
	void setUniform(GLint id, const Mat4& value, bool transpose=false) const;
			
	void enable() const;
	void disable() const;

private:
	GLuint glVertexShader;
	GLuint glFragmentShader;
	GLuint glProgram;
	
	GLProgram(const GLchar** vertexShaderTexts, GLsizei vsCount, const GLchar** framentShaderTexts, GLsizei fsCount);
	static std::string loadFile(const std::string& filename);
	
	static void checkAndThrow();
	static void checkAndThrowShader(GLuint shader);
	static void checkAndThrowProgram(GLuint program);

};