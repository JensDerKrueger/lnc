#pragma once

#include "Image.h"
#include "OpenClContext.h"

class Fractal : public Image {
public:
  Fractal(unsigned int w, unsigned int h, cl_device_id clDevice);
  ~Fractal();  
  void compute();
  
private:
  OpenClContext<unsigned char> context;
};
