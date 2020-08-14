#include "GLArray.h"

GLArray::GLArray() {
	GL(glGenVertexArrays(1, &glId));
	GL(glBindVertexArray(glId));
}

GLArray::~GLArray() {
	GL(glDeleteVertexArrays(1, &glId));
}

void GLArray::bind() const {
	GL(glBindVertexArray(glId));
}

void GLArray::connectVertexAttrib(const GLBuffer& buffer, const GLProgram& program, const std::string& variable, size_t elemCount, size_t offset) const {
	bind();
	const GLint location = program.getAttributeLocation(variable.c_str());
	buffer.connectVertexAttrib(location, elemCount, offset);
}

void GLArray::connectIndexBuffer(const GLBuffer& buffer) const {
	bind();
	buffer.bind();	
}
