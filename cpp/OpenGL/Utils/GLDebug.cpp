#include "GLDebug.h"

void errorOut(const char* source, const char* format, ...) {
  const size_t buffSize = 16384;
  char buff[buffSize];
  va_list args;
  va_start(args, format);
#ifdef _WIN32
  _vsnprintf_s(buff, buffSize, sizeof(buff), format, args);
#else
  vsnprintf(buff, sizeof(buff), format, args);
#endif
  va_end(args);
  std::cerr << source << " " << buff << std::endl;
}
