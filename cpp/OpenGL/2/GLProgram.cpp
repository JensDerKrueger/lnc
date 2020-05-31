#include <fstream>
#include <sstream>

#include "GLProgram.h"


GLProgram::GLProgram(const GLchar** vertexShaderTexts, GLsizei vsCount, const GLchar** framentShaderTexts, GLsizei fsCount) :
	glVertexShader(0),
	glFragmentShader(0),
	glProgram(0)	
{
	glVertexShader = glCreateShader(GL_VERTEX_SHADER); checkAndThrow();
	glShaderSource(glVertexShader, vsCount, vertexShaderTexts, NULL); checkAndThrow();
	glCompileShader(glVertexShader); checkAndThrow();
		
	glFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); checkAndThrow();
	glShaderSource(glFragmentShader, vsCount, framentShaderTexts, NULL); checkAndThrow();
	glCompileShader(glFragmentShader); checkAndThrow();
	
	glProgram = glCreateProgram(); checkAndThrow();
	glAttachShader(glProgram, glVertexShader); checkAndThrow();
	glAttachShader(glProgram, glFragmentShader); checkAndThrow();
	glLinkProgram(glProgram); checkAndThrow();
}


GLProgram GLProgram::createFromFile(const std::string& vs, const std::string& fs) {
	return createFromFiles(std::vector<std::string>{vs},std::vector<std::string>{fs});
}

GLProgram GLProgram::createFromString(const std::string& vs, const std::string& fs) {
	return createFromStrings(std::vector<std::string>{vs},std::vector<std::string>{fs});
}

GLProgram GLProgram::createFromFiles(const std::vector<std::string>& vs, const std::vector<std::string>& fs) {
	std::vector<std::string> vsText{};
	std::vector<std::string> fsText{};
	
	for (std::string s : vs) {
		vsText.push_back(loadFile(s));
	}
	for (std::string s : fs) {
		fsText.push_back(loadFile(s));
	}

	return createFromStrings(vsText,fsText);
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

std::string GLProgram::loadFile(const std::string& filename) {
	std::ifstream shaderFile(filename);
	if (!shaderFile) {
		throw ProgramException(std::string("Unable to open file ") + filename);
	}
	std::string str{};
	std::string all{};
	while (std::getline(shaderFile,str)) {
		all += str + "\n";
	}
	return all;
}

void GLProgram::checkAndThrow() {
	GLenum e = glGetError();
	if (e != GL_NO_ERROR) {
		std::stringstream s;
		s << "An openGL error occured:" << e;
		throw ProgramException{s.str()};
	}
}

GLint GLProgram::getUniformLocation(const std::string& name) const {
	return glGetUniformLocation(glProgram,name.c_str());
}

GLint GLProgram::getAttribLocation(const std::string& name) const {
	return glGetAttribLocation(glProgram,name.c_str());
}

void GLProgram::use() const {
	glUseProgram(glProgram);	
}
