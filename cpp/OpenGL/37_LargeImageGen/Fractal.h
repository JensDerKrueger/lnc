#pragma once

#include <vector>

#include <OpenClUtils.h>

class Fractal {
public:
  Fractal(uint32_t w, uint32_t h,
          int64_t fullW, int64_t fullH,
          int64_t offsetX, int64_t offsetY,
          cl_device_id clDevice);
  ~Fractal();  
  void compute();
  
  uint32_t getWidth() const {return w;}
  uint32_t getHeight() const {return h;}
  const std::vector<uint8_t>& getData() const {return target;}
  
  void setOffset(int64_t offsetX, int64_t offsetY);
  
private:
  uint32_t w;
  uint32_t h;
  std::vector<uint8_t> target;
  OpenClContext<unsigned char> context;
};
