#include <sstream>
#include <fstream>
#include <iostream>

#include "GLProgram.h"

GLuint GLProgram::createShader(GLenum type, const GLchar** src, GLsizei count) {
	if (count==0) return 0;
	GLuint s = glCreateShader(type); checkAndThrow();
	glShaderSource(s, count, src, NULL); checkAndThrow();
	glCompileShader(s); checkAndThrowShader(s);
	return s;
}

GLProgram::GLProgram(const GLchar** vertexShaderTexts, GLsizei vsCount, const GLchar** framentShaderTexts, GLsizei fsCount, const GLchar** geometryShaderTexts, GLsizei gsCount) :
	glVertexShader(0),
	glFragmentShader(0),
	glGeometryShader(0),
	glProgram(0)
{
	
	glVertexShader = createShader(GL_VERTEX_SHADER, vertexShaderTexts, vsCount);
	glFragmentShader = createShader(GL_FRAGMENT_SHADER, framentShaderTexts, fsCount);
	glGeometryShader = createShader(GL_GEOMETRY_SHADER, geometryShaderTexts, gsCount);
	

	glProgram = glCreateProgram(); checkAndThrow();
	if (glVertexShader) glAttachShader(glProgram, glVertexShader); checkAndThrow();
	if (glFragmentShader) glAttachShader(glProgram, glFragmentShader); checkAndThrow();
	if (glGeometryShader) glAttachShader(glProgram, glGeometryShader); checkAndThrow();
	glLinkProgram(glProgram); checkAndThrowProgram(glProgram);
}

GLProgram::~GLProgram() {
	glDeleteShader(glVertexShader);
	glDeleteShader(glFragmentShader);
	glDeleteProgram(glProgram);
}

GLProgram GLProgram::createFromFiles(const std::vector<std::string>& vs, const std::vector<std::string>& fs, const std::vector<std::string>& gs) {
	std::vector<std::string> vsTexts;
	for (const std::string f : vs) {
		vsTexts.push_back(loadFile(f));
	}
	std::vector<std::string> fsTexts;
	for (const std::string f : fs) {
		fsTexts.push_back(loadFile(f));
	}
	std::vector<std::string> gsTexts;
	for (const std::string f : gs) {
		if (!f.empty())		
			gsTexts.push_back(loadFile(f));
	}
	return createFromStrings(vsTexts,fsTexts,gsTexts);
}

GLProgram GLProgram::createFromStrings(const std::vector<std::string>& vs, const std::vector<std::string>& fs, const std::vector<std::string>& gs) {
	std::vector<const GLchar*> vertexShaderTexts;
	for (const std::string& s : vs)
		vertexShaderTexts.push_back(s.c_str());

	std::vector<const GLchar*> framentShaderTexts;
	for (const std::string& s : fs)
		framentShaderTexts.push_back(s.c_str());
		
	std::vector<const GLchar*> geometryShaderTexts;
	for (const std::string& s : gs)
		if (!s.empty())
			geometryShaderTexts.push_back(s.c_str());

	
	return {vertexShaderTexts.data(), GLsizei(vertexShaderTexts.size()), framentShaderTexts.data(), GLsizei(framentShaderTexts.size()), geometryShaderTexts.data(), GLsizei(geometryShaderTexts.size())};
}

GLProgram GLProgram::createFromFile(const std::string& vs, const std::string& fs, const std::string& gs) {
	return createFromFiles(std::vector<std::string>{vs}, std::vector<std::string>{fs}, std::vector<std::string>{gs});
}

GLProgram GLProgram::createFromString(const std::string& vs, const std::string& fs, const std::string& gs) {
	return createFromStrings(std::vector<std::string>{vs}, std::vector<std::string>{fs}, std::vector<std::string> {gs});
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

void GLProgram::setTexture(GLint id, const GLTexture1D& texture, GLuint unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_1D, texture.getId());
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
