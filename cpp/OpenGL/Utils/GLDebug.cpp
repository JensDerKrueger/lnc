#include "GLEnv.h"
#include "GLDebug.h"

void errorOut(const std::string& statement, const std::string& location,
              uint32_t line, const std::string& file, const GLubyte * errorDesc,
              uint32_t errnum) {
  std::cerr << "GL error calling" << statement << " " << location <<  " " << line
            << " (" << file << "):" <<  errorDesc << " (" << errnum << ")" << std::endl;
}
