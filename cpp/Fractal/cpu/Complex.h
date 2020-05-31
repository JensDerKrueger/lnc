#pragma once

#include <cmath> // for "sqrt"

class Complex {
  public:
     float r;
     float i;

 Complex(float r, float i) :
  r(r),
  i(i)
 {}

  Complex() :
   r(0),
   i(0)
  {}

  Complex sq() {
    return Complex(r*r-i*i, 2*r*i);
  }

  float sq_norm() {
    return r*r+i*i;
  }

  float norm() {
    return sqrt(sq_norm());
  }

  Complex add(Complex other) {
    return Complex(r+other.r, i+other.i);
  }
};
