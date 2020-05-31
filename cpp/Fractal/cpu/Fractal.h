#pragma once

#include "Image.h"

class Fractal : public Image {
public:
  Fractal(unsigned int w, unsigned int h) :
   Image(w,h)
  {
  }
  
  void compute();
};
