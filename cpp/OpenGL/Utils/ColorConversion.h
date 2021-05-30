#pragma once

#include "Vec3.h"
#include "Vec4.h"
#include "Mat4.h"

namespace ColorConversion {
  template <typename T> Vec3t<T> rgbToHsv(const Vec3t<T>& other) {
    const T minComp = std::min(other.r, std::min(other.g,other.b));
    const T maxComp = std::max(other.r, std::max(other.g,other.b));
    const T delta = maxComp - minComp;

    T h = T(0);
    if (maxComp != minComp) {
      if (maxComp == other.r)
        h = fmod((T(60) * ((other.g - other.b) / delta) + T(360)), T(360));
      else if (maxComp == other.g)
        h = fmod((T(60) * ((other.b - other.r) / delta) + T(120)), T(360));
      else if (maxComp == other.b)
        h = fmod((T(60) * ((other.r - other.g) / delta) + T(240)), T(360));
    }

    const T s = (maxComp == T(0)) ? T(0) : (delta / maxComp);
    const T v = maxComp;

    return {h,s,v};
  }

  template <typename T> Vec3t<T> hsvToRgb(const Vec3t<T>& other) {
    const T h = T(int32_t(other.x) % 360) / T(60);
    const T s = std::max(T(0), std::min(T(1), other.y));
    const T v = std::max(T(0), std::min(T(1), other.z));

    if (s == 0) return {v,v,v}; // Achromatic (grey)

    const int32_t i = int32_t(floor(h));
    const T f = h - T(i);
    const T p = v * (T(1) - s);
    const T q = v * (T(1) - s * f);
    const T t = v * (T(1) - s * (T(1) - f));

    switch(i) {
      case 0: return {v,t,p};
      case 1: return {q,v,p};
      case 2: return {p,v,t};
      case 3: return {p,q,v};
      case 4: return {t,p,v};
      default: return {v,p,q};
    }
  }

  template <typename T> Vec3t<T> hslToHsv(const Vec3t<T>& other) {
    const T h = other.x;
    const T s = other.y;
    const T l = other.z;
    
    const T v = s*std::min(l,T(1)-l)+l;
    return {h,(v > T(0)) ?  T(2)-T(2)*l/v : T(0), v};
  }
  template <typename T> Vec3t<T> hsvToHsl(const Vec3t<T>& other) {
    const T h = other.x;
    const T s = other.y;
    const T v = other.z;
    
    T l = v-v*s/T(2);
    T m = std::min(l,T(1)-l);
    return {h,(m > T(0)) ? (v-l)/m : l, l};
  }

    template <typename T> Vec3t<T> rgbToCmy(const Vec3t<T>& other) {
      return {T(1)-other.x,T(1)-other.y,T(1)-other.z};
    }

  template <typename T> Vec4t<T> rgbToCmyk(const Vec3t<T>& other) {
    const Vec3t<T> cmy = rgbToCmy(other);
    const T minVal = std::min(cmy.x,std::min(cmy.y,cmy.z));
    return {cmy-minVal,minVal};
  }

    template <typename T> Vec3t<T> cmyToRgb(const Vec3t<T>& other){
      return {T(1)-other.x,T(1)-other.y,T(1)-other.z};
    }

  template <typename T> Vec3t<T> cmykToRgb(const Vec4& other) {
    return {T(1)-(other.x+other.w),T(1)-(other.y+other.w),T(1)-(other.z+other.w)};
  }
      
  template <typename T> Vec3t<T> rgbToYuv(const Vec3t<T>& other) {
    const Mat4 c{   0.299f,   0.587f,  0.114f,  0.0f,
                   -0.147f,  -0.289f,  0.436f,  0.0f,
                    0.615f,  -0.515f, -0.100f,  0.0f,
                      0.0f,     0.0f,    0.0f,  1.0f
    };
    return Vec3t<T>((c * Vec4t<float>(Vec3t<float>(other),1.0f)).xyz);
  }

  template <typename T> Vec3t<T> yuvToRgb(const Vec3t<T>& other) {
    const Mat4 c{ 1.0f,      0.0f,    1.140f,  0.0f,
                  1.0f,   -0.395f,   -0.581f,  0.0f,
                  1.0f,    2.032f,      0.0f,  0.0f,
                  0.0f,      0.0f,      0.0f,  1.0f
    };
    return Vec3t<T>((c * Vec4t<float>(Vec3t<float>(other),1.0f)).xyz);
  }
}
