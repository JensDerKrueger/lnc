#pragma once 

#include <ostream>
#include <string>
#include <array>

#include "Vec2.h"
#include "Vec3.h"

template <typename T>
union Vec4t {
public:
  std::array<float, 4> e;
  struct { float x; float y; float z; float w;};
  struct { float r; float g; float b; float a;};

  Vec4t<T>():
    e{0,0,0,0}
  {}
  
  Vec4t<T>(float x, float y, float z, float w):
    e{x,y,z,w}
  {}
  
  template <typename U>
  Vec4t<T>(const Vec4t<U>& other) :
    e{T(other.x), T(other.y), T(other.z), T(other.w)}
  {}
  
  template <typename U>
  Vec4t<T>(const Vec3t<U>& other, U w):
    e{T(other.x), T(other.y), T(other.z), T(w)}
  {}
  
  template <typename U>
  Vec4t<T>(const Vec2t<U>& other, U z, U w) :
    e{T(other.x), T(other.y), T(z), T(w)}
  {}
  
  Vec3t<T> xyz() const {
    return {e[0],e[1],e[2]};
  }
  Vec2t<T> xy() const {
    return {e[0],e[1]};
  }

  friend std::ostream& operator<<(std::ostream &os, const Vec4t<T>& v) {os << v.toString() ; return os;}
  const std::string toString() const {
    std::stringstream s;
    s << "[" << e[0] << ", " << e[1] << ", " << e[2] << ", " << e[3] << "]";
    return s.str();
  }
  
  template <typename U>
  Vec4t<T> operator+(const Vec4t<U>& val) const {
    return {e[0]+T(val.e[0]),
            e[1]+T(val.e[1]),
            e[2]+T(val.e[2]),
            e[3]+T(val.e[3])};
  }
  
  template <typename U>
  Vec4t<T> operator-(const Vec4t<U>& val) const {
    return {e[0]-T(val.e[0]),
            e[1]-T(val.e[1]),
            e[2]-T(val.e[2]),
            e[3]-T(val.e[3])};
  }
  
  template <typename U>
  Vec4t<T> operator*(const Vec4t<U>& val) const {
    return {e[0]*T(val.e[0]),
            e[1]*T(val.e[1]),
            e[2]*T(val.e[2]),
            e[3]*T(val.e[3])};
  }
  
  template <typename U>
  Vec4t<T> operator/(const Vec4t<U>& val) const {
    return {e[0]/T(val.e[0]),
            e[1]/T(val.e[1]),
            e[2]/T(val.e[2]),
            e[3]/T(val.e[3])};
  }

  Vec4t<T> operator*(const T& val) const {
    return {e[0]*val, e[1]*val, e[2]*val, e[3]*val};
  }
  
  Vec4t<T> operator/(const T& val) const {
    return {e[0]/val, e[1]/val, e[2]/val, e[3]/val};
  }
  
  template <typename U>
  bool operator == ( const Vec4t<U>& other ) const {
    return e[0] == T(other.e[0]) &&
           e[1] == T(other.e[1]) &&
           e[2] == T(other.e[2]) &&
           e[3] == T(other.e[3]);
  }
  
  template <typename U>
  bool operator != ( const Vec4t<U>& other ) const {
    return e[0] != T(other.e[0]) ||
           e[1] != T(other.e[1]) ||
           e[2] != T(other.e[2]) ||
           e[3] != T(other.e[3]);
  }

  float length() const {
    return sqrt(sqlength());
  }
  
  float sqlength() const {
    return float(e[0]*e[0]+e[1]*e[1]+e[2]*e[2]+e[3]*e[3]);
  }
  
  Vec3t<T> vec3() const {
    return {e[0], e[1], e[2]};
  }
  
  operator float*(void) {return e.data();}
  operator const float*(void) const  {return e.data();}
    
  static float dot(const Vec4t<T>& a, const Vec4t<T>& b) {
    return a.e[0]*b.e[0]+a.e[1]*b.e[1]+a.e[2]*b.e[2]+a.e[3]*b.e[3];
  }
  
  static Vec4t<float> normalize(const Vec4t<float>& a) {
    return a/a.length();
  }
      
  static Vec4t<float> random() {
    return {Rand::rand01(),Rand::rand01(),Rand::rand01(),Rand::rand01()};
  }
};

typedef Vec4t<float> Vec4;
typedef Vec4t<int32_t> Vec4i;
typedef Vec4t<uint32_t> Vec4ui;
