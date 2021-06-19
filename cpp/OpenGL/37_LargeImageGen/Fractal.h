#pragma once

#include <vector>

#include "OpenClContext.h"

class Fractal {
public:
  Fractal(uint32_t w, uint32_t h,
          uint32_t fullW, uint32_t fullH,
          uint32_t offsetX, uint32_t offsetY,
          cl_device_id clDevice);
  ~Fractal();  
  void compute();
  
  uint32_t getWidth() const {return w;}
  uint32_t getHeight() const {return h;}
  const std::vector<uint8_t>& getData() const {return target;}
  
  void setOffset(uint32_t offsetX, uint32_t offsetY);
  
private:
  uint32_t w;
  uint32_t h;
  std::vector<uint8_t> target;
  OpenClContext<unsigned char> context;
};
