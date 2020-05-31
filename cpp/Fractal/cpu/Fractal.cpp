#include "Fractal.h"
#include "Complex.h"

void Fractal::compute() {
  float dx = 2.8f / w;
  float dy = 2.6f / h;

  float xs = -2.1f;
  float ys = -1.3f;

  Complex c;
  for (uint32_t y = 0;y<h;++y) {
    c.i = ys + dy*y;
    for (uint32_t x = 0;x<w;++x) {
      c.r = xs + dx*x;
      Complex d;
      
      uint16_t depth = 0;
      while (depth < 256 && d.sq_norm() < 4) {
        d = d.sq().add(c);
        depth++;
      }
      
      setData(x,y,(uint8_t)depth);
    }
  }
}

