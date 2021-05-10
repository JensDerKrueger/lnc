#include "Fractal.h"

Fractal::Fractal(unsigned int w, unsigned int h, cl_device_id clDevice)
: Image(w,h)
, context(w,h)
{
  std::string code =  "\n" \
  "__kernel void clmain(                       \n" \
  "   __global unsigned char* output,          \n" \
  "   const unsigned int w,                    \n" \
  "   const unsigned int h)                    \n" \
  "{                                           \n" \
  "  const float dx = 2.8f / w;                \n" \
  "  const float dy = 2.6f / h;                \n" \
  "                                            \n" \
  "  const float xs = -2.1f;                   \n" \
  "  const float ys = -1.3f;                   \n" \
  "                                            \n" \
  "  const int x = get_global_id(0);           \n" \
  "  const int y = get_global_id(1);           \n" \
  "                                            \n" \
  "  float cR = xs + dx*x, cI = ys + dy*y;     \n" \
  "  float dR = 0, dI = 0;                     \n" \
  "  unsigned int depth = 0;                   \n" \
  "  while (depth < 256 && dR*dR+dI*dI < 4) {  \n" \
  "    float t = dR;                           \n" \
  "    dR = dR*dR-dI*dI+cR;                    \n" \
  "    dI = 2*t*dI+cI;                         \n" \
  "    depth++;                                \n" \
  "  }                                         \n" \
  "  output[x+y*w] = depth;                    \n" \
  "}                                           \n" ;
      
  context.init(clDevice);
  context.setProgramCode(code, false);
  context.setParam(0, sizeof(unsigned int), &w);
  context.setParam(1, sizeof(unsigned int), &h);
}

void Fractal::compute() {
  context.run();
  context.getOutput((unsigned char*)(getDataVector().data()));
}

Fractal::~Fractal() {
  context.destroy();
}
