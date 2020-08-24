#include "GLEnv.h"
#include "GLDebug.h"

std::string errorString(GLenum glerr) {
  switch (glerr) {
    case GL_NO_ERROR: return "GL_NO_ERROR";
    case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
    default: return "Unknown Error";
  }
}

void errorOut(const std::string& statement, const std::string& location,
              uint32_t line, const std::string& file, uint32_t errnum) {
  std::cerr << "GL error calling " << statement << " " << location <<  " " << line
            << " (" << file << "):" <<  errorString(errnum) << " (" << errnum << ")" << std::endl;
}
