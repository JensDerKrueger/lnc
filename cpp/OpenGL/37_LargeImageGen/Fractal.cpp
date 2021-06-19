#include "Fractal.h"

Fractal::Fractal(uint32_t w, uint32_t h,
                 uint32_t fullW, uint32_t fullH,
                 uint32_t offsetX, uint32_t offsetY,
                 cl_device_id clDevice)
: w(w)
, h(h)
, target(w*h)
, context(w,h)
{
  std::string code =  "\n" \
  "__kernel void clmain(                       \n" \
  "   __global unsigned char* output,          \n" \
  "   const unsigned int w,                    \n" \
  "   const unsigned int h,                    \n" \
  "   const unsigned int fullW,                \n" \
  "   const unsigned int fullH,                \n" \
  "   const unsigned int offsetX,              \n" \
  "   const unsigned int offsetY)              \n" \
  "{                                           \n" \
  "  const float dx = 2.8f / fullW;            \n" \
  "  const float dy = 2.6f / fullH;            \n" \
  "                                            \n" \
  "  const float xs = -2.1f;                   \n" \
  "  const float ys = -1.3f;                   \n" \
  "                                            \n" \
  "  const int x = get_global_id(0);           \n" \
  "  const int y = get_global_id(1);           \n" \
  "                                            \n" \
  "  float cR = xs + dx*(x+offsetX);           \n" \
  "  float cI = ys + dy*(y+offsetY);           \n" \
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
  context.setParam(0, sizeof(uint32_t), &w);
  context.setParam(1, sizeof(uint32_t), &h);
  context.setParam(2, sizeof(uint32_t), &fullW);
  context.setParam(3, sizeof(uint32_t), &fullH);
  context.setParam(4, sizeof(uint32_t), &offsetX);
  context.setParam(5, sizeof(uint32_t), &offsetY);
}

void Fractal::setOffset(uint32_t offsetX, uint32_t offsetY) {
  context.setParam(4, sizeof(uint32_t), &offsetX);
  context.setParam(5, sizeof(uint32_t), &offsetY);
}

void Fractal::compute() {
  context.run();
  context.getOutput((unsigned char*)(target.data()));
}

Fractal::~Fractal() {
  context.destroy();
}
