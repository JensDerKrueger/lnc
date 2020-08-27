#pragma once

#include "Vec3.h"
#include "Mat4.h"

class Quaternion {
public:
  Quaternion():
    x(0),
    y(0),
    z(0),
    w(0)
  {}

  Quaternion(const Vec3 v, float w):
    x(v.x()),
    y(v.y()),
    z(v.z()),
    w(w)
  {}

  Quaternion(float x, float y, float z, float w):
    x(x),
    y(y),
    z(z),
    w(w)
  {}
  
  Quaternion(const Quaternion& other) :
    x(other.x),
    y(other.y),
    z(other.z),
    w(other.w)
  {}

  Mat4 computeRotation() const {
    float n, s;
    float xs, ys, zs;
    float wx, wy, wz;
    float xx, xy, xz;
    float yy, yz, zz;
    
    n = (x * x) + (y * y) + (z * z) + (w * w);
    s = (n > 0.0f) ? (2.0f / n) : 0.0f;
    
    xs = x * s;
    ys = y * s;
    zs = z * s;
    wx = w * xs;
    wy = w * ys;
    wz = w * zs;
    xx = x * xs;
    xy = x * ys;
    xz = x * zs;
    yy = y * ys;
    yz = y * zs;
    zz = z * zs;
    
    return Mat4(1.0f - (yy + zz), xy - wz,          xz + wy,          0,
                xy + wz,          1.0f - (xx + zz), yz - wx,          0,
                xz - wy,          yz + wx,          1.0f - (xx + yy), 0,
                0,                0,                0,                1);
  }
  
private:
  float x;
  float y;
  float z;
  float w;
};
