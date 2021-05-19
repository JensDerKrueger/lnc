#pragma once 

#include <ostream>
#include <sstream>
#include <string>
#include <array>
#include <cmath>

#include "Rand.h"

template <typename T>
union Vec3t {
public:
  std::array<T, 3> e;
  struct { T x; T y; T z; };
  struct { T r; T g; T b; };

  Vec3t() :
    e{0,0,0}
  {}
  
  Vec3t(T x, T y, T z) :
    e{x,y,z}
  {}

  template <typename U>
  Vec3t(const Vec3t<U>& other) :
    e{T(other.x), T(other.y), T(other.z)}
  {}
  
  template <typename U>
  friend std::ostream& operator<<(std::ostream &os, const Vec3t<U>& v) {os << v.toString() ; return os;}
  const std::string toString() const {
    std::stringstream s;
    s << "[" << e[0] << ", " << e[1] << ", " << e[2] << "]";
    return s.str();
  }
   
  template <typename U>
  Vec3t<T> operator+(const Vec3t<U>& val) const {
    return Vec3t<T>{e[0]+T(val.e[0]),
                    e[1]+T(val.e[1]),
                    e[2]+T(val.e[2])};
  }
  
  template <typename U>
  Vec3t<T> operator-(const Vec3t<U>& val) const {
    return Vec3t<T>{e[0]-T(val.e[0]),
                    e[1]-T(val.e[1]),
                    e[2]-T(val.e[2])};
  }
  
  template <typename U>
  Vec3t<T> operator*(const Vec3t<U>& val) const{
    return Vec3t<T>{e[0]*T(val.e[0]),
                    e[1]*T(val.e[1]),
                    e[2]*T(val.e[2])};
  }
  
  template <typename U>
  Vec3t<T> operator/(const Vec3t<U>& val) const{
    return Vec3t<T>{e[0]/T(val.e[0]),
                    e[1]/T(val.e[1]),
                    e[2]/T(val.e[2])};
  }

  Vec3t<T> operator*(const T& val) const {
    return Vec3t<T>{e[0]*val,e[1]*val,e[2]*val};
  }
  
  Vec3t<T> operator/(const T& val) const{
    return Vec3t<T>{e[0]/val,e[1]/val,e[2]/val};
  }
  
  template <typename U>
  bool operator == (const Vec3t<U>& other) const {
    return e[0] == T(other.e[0]) &&
           e[1] == T(other.e[1]) &&
           e[2] == T(other.e[2]);
  }
  
  template <typename U>
  bool operator != (const Vec3t<U>& other) const {
    return e[0] != T(other.e[0]) ||
           e[1] != T(other.e[1]) ||
           e[2] != T(other.e[2]);
  }

  float length() const {
    return sqrt(sqlength());
  }
  
  float sqlength() const {
    return float(e[0]*e[0]+e[1]*e[1]+e[2]*e[2]);
  }
  
  operator T*(void) {return e.data();}
  operator const T*(void) const  {return e.data();}
    
  static float dot(const Vec3t<T>& a, const Vec3t<T>& b) {
    return a.e[0]*b.e[0]+a.e[1]*b.e[1]+a.e[2]*b.e[2];
  }
  
  static Vec3t<T> cross(const Vec3t<T>& a, const Vec3t<T>& b) {
    return Vec3t<T>{a.e[1] * b.e[2] - a.e[2] * b.e[1],
                    a.e[2] * b.e[0] - a.e[0] * b.e[2],
                    a.e[0] * b.e[1] - a.e[1] * b.e[0]};
  }
  
  static Vec3t<T> normalize(const Vec3t<T>& a) {
    const float l = a.length();
    return (l != 0.0f) ? a/T(l) : Vec3t<T>{0,0,0};
  }
  
  static Vec3t<float> reflect(const Vec3t<float>& a, const Vec3t<float>& n) {
    return a-n*dot(a,n)*2;
  }
  
  static Vec3t<float> refract(const Vec3t<float>& a, const Vec3t<float>& n,
                          const float index) {
    const float cosTheta = fmin(-dot(a, n), 1.0f);
    const Vec3t<float> rOutParallel{(a + n*cosTheta) * index};
    const Vec3t<float> rOutPerpendicular{n * -sqrt(1.0f - fmin(rOutParallel.sqlength(), 1.0f))};
    return rOutParallel + rOutPerpendicular;
  }
      
  static Vec3t<T> minV(const Vec3t<T>& a, const Vec3t<T>& b) {
    return {std::min(a.x,b.x), std::min(a.y,b.y), std::min(a.z,b.z)};
  }
  
  static Vec3t<T> maxV(const Vec3t<T>& a, const Vec3t<T>& b) {
    return {std::max(a.x,b.x), std::max(a.y,b.y), std::max(a.z,b.z)};
  }
  
  static Vec3t<float> random() {
    return {Rand::rand01(),Rand::rand01(),Rand::rand01()};
  }
  
  static Vec3t<float> randomPointInSphere() {
    while (true) {
      Vec3t<float> p{Rand::rand11(),Rand::rand11(),Rand::rand11()};
      if (p.sqlength() > 1) continue;
      return p;
    }
  }
  
  static Vec3t<float> randomPointInHemisphere() {
    while (true) {
      Vec3t<float> p{Rand::rand01(),Rand::rand01(),Rand::rand01()};
      if (p.sqlength() > 1) continue;
      return p;
    }
  }
  
  static Vec3t<float> randomPointInDisc() {
    while (true) {
      Vec3t<float> p{Rand::rand11(),Rand::rand11(),0};
      if (p.sqlength() > 1) continue;
      return p;
    }
  }
  
  static Vec3t<float> randomUnitVector() {
    const float a = Rand::rand0Pi();
    const float z = Rand::rand11();
    const float r = sqrt(1.0f - z*z);
    return {r*cosf(a), r*sinf(a), z};
  }
  
  static Vec3t<float> hsvToRgb(const Vec3t<float>& other) {
    // Make sure our arguments stay in-range
    const float h = float(int(other.x) % 360) / 60;
    const float s = std::max(0.0f, std::min(1.0f, other.y));
    const float v = std::max(0.0f, std::min(1.0f, other.z));

    if(s == 0) return {v,v,v}; // Achromatic (grey)

    const int i = int(floor(h));
    const float f = h - i;
    const float p = v * (1 - s);
    const float q = v * (1 - s * f);
    const float t = v * (1 - s * (1 - f));

    switch(i) {
      case 0: return {v,t,p};
      case 1: return {q,v,p};
      case 2: return {p,v,t};
      case 3: return {p,q,v};
      case 4: return {t,p,v};
      default: return {v,p,q};
    }
  }
  		
};

typedef Vec3t<float> Vec3;
typedef Vec3t<int32_t> Vec3i;
typedef Vec3t<uint32_t> Vec3ui;
