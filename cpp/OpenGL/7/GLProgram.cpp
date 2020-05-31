#include <sstream>
#include <fstream>
#include <iostream>

#include "GLProgram.h"

GLProgram::GLProgram(const GLchar** vertexShaderTexts, GLsizei vsCount, const GLchar** framentShaderTexts, GLsizei fsCount) :
	glVertexShader(0),
	glFragmentShader(0),
	glProgram(0)
{
	glVertexShader = glCreateShader(GL_VERTEX_SHADER); checkAndThrow();
	glShaderSource(glVertexShader, vsCount, vertexShaderTexts, NULL); checkAndThrow();
	glCompileShader(glVertexShader); checkAndThrowShader(glVertexShader);
	
	glFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); checkAndThrow();
	glShaderSource(glFragmentShader, fsCount, framentShaderTexts, NULL); checkAndThrow();
	glCompileShader(glFragmentShader); checkAndThrowShader(glFragmentShader);

	glProgram = glCreateProgram(); checkAndThrow();
	glAttachShader(glProgram, glVertexShader); checkAndThrow();
	glAttachShader(glProgram, glFragmentShader); checkAndThrow();
	glLinkProgram(glProgram); checkAndThrowProgram(glProgram);
}

GLProgram::~GLProgram() {
	glDeleteShader(glVertexShader);
	glDeleteShader(glFragmentShader);
	glDeleteProgram(glProgram);
}

GLProgram GLProgram::createFromFiles(const std::vector<std::string>& vs, const std::vector<std::string>& fs) {
	std::vector<std::string> vsTexts;
	for (const std::string f : vs) {
		vsTexts.push_back(loadFile(f));
	}
	std::vector<std::string> fsTexts;
	for (const std::string f : fs) {
		fsTexts.push_back(loadFile(f));
	}
	return createFromStrings(vsTexts,fsTexts);
}

GLProgram GLProgram::createFromStrings(const std::vector<std::string>& vs, const std::vector<std::string>& fs) {
	std::vector<const GLchar*> vertexShaderTexts(vs.size());
	std::vector<const GLchar*> framentShaderTexts(fs.size());
		
	for (size_t i = 0;i<vs.size();++i) {
		vertexShaderTexts[i] = vs[i].c_str();
	}
	for (size_t i = 0;i<fs.size();++i) {
		framentShaderTexts[i] = fs[i].c_str();
	}
	
	return {vertexShaderTexts.data(), GLsizei(vs.size()), framentShaderTexts.data(), GLsizei(fs.size())};
}

GLProgram GLProgram::createFromFile(const std::string& vs, const std::string& fs) {
	return createFromFiles(std::vector<std::string>{vs}, std::vector<std::string>{fs});
}

GLProgram GLProgram::createFromString(const std::string& vs, const std::string& fs) {
	return createFromStrings(std::vector<std::string>{vs}, std::vector<std::string>{fs});
}

std::string GLProgram::loadFile(const std::string& filename) {
	std::ifstream shaderFile{filename};
	if (!shaderFile) {
		throw ProgramException{std::string("Unable to open file ") +  filename};
	}
	std::string str;
	std::string fileContents;
	while (std::getline(shaderFile, str)) {
		fileContents += str + "\n";
	} 
	return fileContents;
}

GLint GLProgram::getAttributeLocation(const std::string& id) const {
	GLint l = glGetAttribLocation(glProgram, id.c_str());
	checkAndThrow();	
	if(l == -1)
		throw ProgramException{std::string("Can't find attribute ") +  id};	
	return l;
}

GLint GLProgram::getUniformLocation(const std::string& id) const {
	GLint l = glGetUniformLocation(glProgram, id.c_str());
	checkAndThrow();
	if(l == -1)
		throw ProgramException{std::string("Can't find uniform ") +  id};	
	return l;
}

void GLProgram::enable() const {
	glUseProgram(glProgram);
}

void GLProgram::disable() const {
	glUseProgram(0);
}

void GLProgram::setUniform(GLint id, float value) const {
	glUniform1f(id, value);	
}

void GLProgram::setUniform(GLint id, const Vec3& value) const {
	glUniform3fv(id, 1, value);	
}

void GLProgram::setUniform(GLint id, const Mat4& value, bool transpose) const {
	glUniformMatrix4fv(id, 1, transpose, value);
}

void GLProgram::setTexture(GLint id, const GLTexture2D& texture, GLuint unit) const {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, texture.getId());
	glUniform1i(id, unit);
}

void GLProgram::checkAndThrow() {
	GLenum e = glGetError();
	if (e != GL_NO_ERROR) {
		std::stringstream s;
		s << "An openGL error occured:" << e;
		throw ProgramException{s.str()};
	}	
}

void GLProgram::checkAndThrowShader(GLuint shader) {
	GLint success[1] = { GL_TRUE };
	glGetShaderiv(shader, GL_COMPILE_STATUS, success);
	if(success[0] == GL_FALSE) {
		GLint log_length{0};
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
		log_length = std::min(static_cast<GLint>(4096), log_length);
		std::vector<GLchar> log(log_length);
		glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), NULL, log.data());
		std::string str{log.data()};
		throw ProgramException{str};
	}
}

void GLProgram::checkAndThrowProgram(GLuint program) {
	GLint linked{GL_TRUE};
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if(linked != GL_TRUE) {
		GLint log_length{0};
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
		log_length = std::min(static_cast<GLint>(4096), log_length);
		std::vector<GLchar> log(log_length);
		glGetProgramInfoLog(program, static_cast<GLsizei>(log.size()), NULL, log.data());
		std::string str{log.data()};
		throw ProgramException{str};
	}		
}