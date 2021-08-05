#include "Fractal.h"

Fractal::Fractal(uint32_t w, uint32_t h,
                 int64_t fullW, int64_t fullH,
                 int64_t offsetX, int64_t offsetY,
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
  "   const long fullW,                        \n" \
  "   const long fullH,                        \n" \
  "   const long offsetX,                      \n" \
  "   const long offsetY)                      \n" \
  "{                                           \n" \
  "  const double dx = 2.8 / fullW;            \n" \
  "  const double dy = 2.6 / fullH;            \n" \
  "                                            \n" \
  "  const double xs = -2.1;                   \n" \
  "  const double ys = -1.3;                   \n" \
  "                                            \n" \
  "  const int x = get_global_id(0);           \n" \
  "  const int y = get_global_id(1);           \n" \
  "                                            \n" \
  "  double cR = xs + dx*(x+offsetX);          \n" \
  "  double cI = ys + dy*(y+offsetY);          \n" \
  "  double dR = 0, dI = 0;                    \n" \
  "  unsigned int depth = 0;                   \n" \
  "  //Cartenoid Check:                        \n" \
  "  float xx = cR;                            \n" \
  "  float yy = cI;                            \n" \
  "  xx = xx - 0.25f;                          \n" \
  "  yy = yy * yy;                             \n" \
  "  float q = xx * xx + yy;                   \n" \
  "  q = q * (q + xx);                         \n" \
  "  if (!((q * 4.0f < yy) || ((xx + 1.25f) *  \n" \
  "       (xx + 1.25f) + yy < 1.0f / 16.0f)))  \n" \
  "  while (depth < 256 && dR*dR+dI*dI < 4) {  \n" \
  "    double t = dR;                          \n" \
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
  context.setParam(2, sizeof(int64_t), &fullW);
  context.setParam(3, sizeof(int64_t), &fullH);
  context.setParam(4, sizeof(int64_t), &offsetX);
  context.setParam(5, sizeof(int64_t), &offsetY);
}

void Fractal::setResolution(int64_t fullW, int64_t fullH) {
  context.setParam(2, sizeof(int64_t), &fullW);
  context.setParam(3, sizeof(int64_t), &fullH);
}

void Fractal::setOffset(int64_t offsetX, int64_t offsetY) {
  context.setParam(4, sizeof(int64_t), &offsetX);
  context.setParam(5, sizeof(int64_t), &offsetY);
}

void Fractal::compute() {
  context.run();
  context.getOutput((unsigned char*)(target.data()));
}

Fractal::~Fractal() {
  context.destroy();
}
