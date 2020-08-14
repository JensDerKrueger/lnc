#pragma once

#include <iostream>

#ifndef NDEBUG

// under some circumstances the glError loops below do not
// terminate, either glError itself causes an erro or does
// not reset the error state. Neither should happen, but
// still do
#define MAX_GL_ERROR_COUNT 10

#define T_ERROR(...)                                              \
  do {                                                            \
    errorOut(__func__, __VA_ARGS__);   \
  } while(0)


void errorOut(const char* source, const char* format, ...);

# define GL(stmt)                                                      \
  do {                                                                 \
    GLenum glerr;                                                      \
    uint32_t counter = 0;                                         \
    while((glerr = glGetError()) != GL_NO_ERROR) {                     \
      T_ERROR("GL error calling %s before line %u (%s): %s (%#x)",     \
              #stmt, __LINE__, __FILE__,                               \
              gluErrorString(glerr),                                   \
              static_cast<uint32_t>(glerr));                           \
      counter++;                                                      \
      if (counter > MAX_GL_ERROR_COUNT) break;                        \
    }                                                                  \
    stmt;                                                              \
    counter = 0;                                                      \
    while((glerr = glGetError()) != GL_NO_ERROR) {                     \
      T_ERROR("'%s' on line %u (%s) caused GL error: %s (%#x)", #stmt, \
              __LINE__, __FILE__,                                      \
              gluErrorString(glerr),                                   \
              static_cast<uint32_t>(glerr));                           \
      counter++;                                                      \
      if (counter > MAX_GL_ERROR_COUNT) break;                        \
    }                                                                  \
  } while(0)


#else
# define GL(stmt) do { stmt; } while(0)
#endif
