/*************************************
*        Quick und Dirty             *
*       OpenCL Fractal Test          *
*                                    *
*        (c) 2016 Jens Krüger        *
*  This code is in the public domain *
**************************************/

#pragma once

#include "Image.h"
#include "OpenClContext.h"

class Fractal : public Image {
public:
  Fractal(unsigned int w, unsigned int h);
  ~Fractal();
  
  void compute();
  
private:
  OpenClContext<unsigned char> context;
};
