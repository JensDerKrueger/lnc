#include <sstream>
#include <fstream>
#include <iostream>

#include "GLProgram.h"
#include "GLDebug.h"

GLProgram::GLProgram(const GLProgram& other) :
  GLProgram(other.vertexShaderStrings, other.fragmentShaderStrings, other.geometryShaderStrings)
{
}

GLProgram& GLProgram::operator=(const GLProgram& other) {
  GL(glDeleteShader(glVertexShader));
  GL(glDeleteShader(glFragmentShader));
  GL(glDeleteShader(glGeometryShader));
  GL(glDeleteProgram(glProgram));
  programFromVectors(other.vertexShaderStrings, other.fragmentShaderStrings, other.geometryShaderStrings);
  return *this;
}

GLuint GLProgram::createShader(GLenum type, const GLchar** src, GLsizei count) {
	if (count==0) return 0;
	GLuint s = glCreateShader(type); checkAndThrow();
	glShaderSource(s, count, src, NULL); checkAndThrow();
	glCompileShader(s); checkAndThrowShader(s);
	return s;
}

GLProgram::GLProgram(std::vector<std::string> vertexShaderStrings, std::vector<std::string> fragmentShaderStrings, std::vector<std::string> geometryShaderStrings):
  glVertexShader(0),
  glFragmentShader(0),
  glGeometryShader(0),
  glProgram(0),
  vertexShaderStrings(vertexShaderStrings),
  fragmentShaderStrings(fragmentShaderStrings),
  geometryShaderStrings(geometryShaderStrings)
{
  programFromVectors(vertexShaderStrings, fragmentShaderStrings, geometryShaderStrings);
}

GLProgram::~GLProgram() {
	GL(glDeleteShader(glVertexShader));
	GL(glDeleteShader(glFragmentShader));
  GL(glDeleteShader(glGeometryShader));
	GL(glDeleteProgram(glProgram));
}

GLProgram GLProgram::createFromFiles(const std::vector<std::string>& vs, const std::vector<std::string>& fs, const std::vector<std::string>& gs) {
	std::vector<std::string> vsTexts;
	for (const std::string& f : vs) {
		vsTexts.push_back(loadFile(f));
	}
	std::vector<std::string> fsTexts;
	for (const std::string& f : fs) {
		fsTexts.push_back(loadFile(f));
	}
	std::vector<std::string> gsTexts;
	for (const std::string& f : gs) {
		if (!f.empty())		
			gsTexts.push_back(loadFile(f));
	}
	return createFromStrings(vsTexts,fsTexts,gsTexts);
}

GLProgram GLProgram::createFromStrings(const std::vector<std::string>& vs, const std::vector<std::string>& fs, const std::vector<std::string>& gs) {
	return {vs,fs,gs};
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
	GL(glUseProgram(glProgram));
}

void GLProgram::disable() const {
	GL(glUseProgram(0));
}

void GLProgram::setUniform(GLint id, float value) const {
	GL(glUniform1f(id, value));
}

void GLProgram::setUniform(GLint id, const Vec2& value) const {
  GL(glUniform2fv(id, 1, value));
}

void GLProgram::setUniform(GLint id, const Vec3& value) const {
	GL(glUniform3fv(id, 1, value));
}

void GLProgram::setUniform(GLint id, const Vec4& value) const {
  GL(glUniform4fv(id, 1, value));
}

void GLProgram::setUniform(GLint id, int value) const {
  GL(glUniform1i(id, value));
}

void GLProgram::setUniform(GLint id, const Vec2i& value) const {
  GL(glUniform2iv(id, 1, value));
}

void GLProgram::setUniform(GLint id, const Mat4& value, bool transpose) const {
	// since OpenGL matrices are usuall expcted
  // column major but our matrices are row major
  // hence, we invert the transposition flag
  GL(glUniformMatrix4fv(id, 1, !transpose, value));
}

void GLProgram::setTexture(GLint id, const GLTexture1D& texture, GLuint unit) const {
  GL(glActiveTexture(GL_TEXTURE0 + unit));
  GL(glBindTexture(GL_TEXTURE_1D, texture.getId()));
  GL(glUniform1i(id, unit));
}

void GLProgram::setTexture(GLint id, const GLTexture2D& texture, GLuint unit) const {
	GL(glActiveTexture(GL_TEXTURE0 + unit));
	GL(glBindTexture(GL_TEXTURE_2D, texture.getId()));
	GL(glUniform1i(id, unit));
}

void GLProgram::setTexture(GLint id, const GLTexture3D& texture, GLuint unit) const {
  GL(glActiveTexture(GL_TEXTURE0 + unit));
  GL(glBindTexture(GL_TEXTURE_3D, texture.getId()));
  GL(glUniform1i(id, unit));
}

void GLProgram::unsetTexture1D(GLuint unit) const {
  GL(glActiveTexture(GL_TEXTURE0 + unit));
  GL(glBindTexture(GL_TEXTURE_1D, 0));
}

void GLProgram::unsetTexture2D(GLuint unit) const {
  GL(glActiveTexture(GL_TEXTURE0 + unit));
  GL(glBindTexture(GL_TEXTURE_2D, 0));
}

void GLProgram::unsetTexture3D(GLuint unit) const {
  GL(glActiveTexture(GL_TEXTURE0 + unit));
  GL(glBindTexture(GL_TEXTURE_3D, 0));
}

void GLProgram::checkAndThrow() {
	GLenum e = glGetError();
	if (e != GL_NO_ERROR) {
		std::stringstream s;
		s << "An openGL error occured:" << errorString(e);
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

void GLProgram::programFromVectors(std::vector<std::string> vs, std::vector<std::string> fs, std::vector<std::string> gs) {
  vertexShaderStrings   = vs;
  fragmentShaderStrings = fs;
  geometryShaderStrings = gs;

  std::vector<const GLchar*> vertexShaderTexts;
  for (const std::string& s : vertexShaderStrings)
   vertexShaderTexts.push_back(s.c_str());

  std::vector<const GLchar*> fragmentShaderTexts;
  for (const std::string& s : fragmentShaderStrings)
   fragmentShaderTexts.push_back(s.c_str());
   
  std::vector<const GLchar*> geometryShaderTexts;
  for (const std::string& s : geometryShaderStrings)
   if (!s.empty())
     geometryShaderTexts.push_back(s.c_str());

  glVertexShader = createShader(GL_VERTEX_SHADER, vertexShaderTexts.data(), GLsizei(vertexShaderTexts.size()));
  glFragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderTexts.data(), GLsizei(fragmentShaderTexts.size()));
  glGeometryShader = createShader(GL_GEOMETRY_SHADER, geometryShaderTexts.data(), GLsizei(geometryShaderTexts.size()));

  glProgram = glCreateProgram(); checkAndThrow();
  if (glVertexShader) {glAttachShader(glProgram, glVertexShader); checkAndThrow();}
  if (glFragmentShader) {glAttachShader(glProgram, glFragmentShader); checkAndThrow();}
  if (glGeometryShader) {glAttachShader(glProgram, glGeometryShader); checkAndThrow();}
  glLinkProgram(glProgram); checkAndThrowProgram(glProgram);
}


void GLProgram::setUniform(const std::string& id, float value) const {
  setUniform(getUniformLocation(id), value);
}

void GLProgram::setUniform(const std::string& id, const Vec2& value) const {
  setUniform(getUniformLocation(id), value);
}

void GLProgram::setUniform(const std::string& id, const Vec3& value) const {
  setUniform(getUniformLocation(id), value);
}

void GLProgram::setUniform(const std::string& id, const Vec4& value) const {
  setUniform(getUniformLocation(id), value);
}

void GLProgram::setUniform(const std::string& id, int value) const {
  setUniform(getUniformLocation(id), value);
}

void GLProgram::setUniform(const std::string& id, const Vec2i& value) const {
  setUniform(getUniformLocation(id), value);
}

void GLProgram::setUniform(const std::string& id, const Mat4& value, bool transpose) const {
  setUniform(getUniformLocation(id), value, transpose);
}


void GLProgram::setTexture(const std::string& id, const GLTexture1D& texture, GLuint unit) const {
  setTexture(getUniformLocation(id), texture, unit);
}

void GLProgram::setTexture(const std::string& id, const GLTexture2D& texture, GLuint unit) const {
  setTexture(getUniformLocation(id), texture, unit);
}

void GLProgram::setTexture(const std::string& id, const GLTexture3D& texture, GLuint unit) const {
  setTexture(getUniformLocation(id), texture, unit);
}

