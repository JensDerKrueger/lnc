#pragma once 

#include <ostream>
#include <sstream>
#include <string>
#include <array>
#include <cmath>

#include "Rand.h"
#include "Vec2.h"

template <typename T>
union Vec3t {
public:
  std::array<T, 3> e;
  struct { T x; T y; T z; };
  struct { T r; T g; T b; };
  Vec2t<T> xy;
  
  Vec3t() :
    e{0,0,0}
  {}
  
  Vec3t(T x, T y, T z) :
    e{x,y,z}
  {}

  Vec3t(const Vec3t& other) :
    e{other.x, other.y, other.z}
  {}
  
  Vec3t(const Vec2t<T>& other, T z) :
    e{other.x, other.y, z}
  {}

  template <typename U>
  explicit Vec3t(const Vec3t<U>& other) :
    e{T(other.x), T(other.y), T(other.z)}
  {}

  template <typename U>
  explicit Vec3t(const Vec2t<U>& other, T z) :
    e{T(other.x), T(other.y), z}
  {}
  
  const std::string toString() const {
    std::stringstream s;
    s << "[" << e[0] << ", " << e[1] << ", " << e[2] << "]";
    return s.str();
  }
   
  Vec3t operator+(const Vec3t& val) const {
    return Vec3t{e[0]+val.e[0],
                    e[1]+val.e[1],
                    e[2]+val.e[2]};
  }
  
  Vec3t operator-(const Vec3t& val) const {
    return Vec3t{e[0]-val.e[0],
                    e[1]-val.e[1],
                    e[2]-val.e[2]};
  }
  
  Vec3t operator*(const Vec3t& val) const{
    return Vec3t{e[0]*val.e[0],
                    e[1]*val.e[1],
                    e[2]*val.e[2]};
  }
  
  Vec3t operator/(const Vec3t& val) const{
    return Vec3t{e[0]/val.e[0],
                    e[1]/val.e[1],
                    e[2]/val.e[2]};
  }

  Vec3t operator*(const T& val) const {
    return {e[0]*val,e[1]*val,e[2]*val};
  }
  
  Vec3t operator/(const T& val) const{
    return {e[0]/val,e[1]/val,e[2]/val};
  }

  Vec3t operator+(const T& val) const {
    return {e[0]+val,e[1]+val,e[2]+val};
  }
  
  Vec3t operator-(const T& val) const{
    return {e[0]-val,e[1]-val,e[2]-val};
  }
  
  bool operator == (const Vec3t& other) const {
    return e[0] == other.e[0] &&
           e[1] == other.e[1] &&
           e[2] == other.e[2];
  }
  
  bool operator != (const Vec3t& other) const {
    return e[0] != other.e[0] ||
           e[1] != other.e[1] ||
           e[2] != other.e[2];
  }

  T length() const {
    return sqrt(sqlength());
  }
  
  T sqlength() const {
    return e[0]*e[0]+e[1]*e[1]+e[2]*e[2];
  }
  
  operator T*(void) {return e.data();}
  operator const T*(void) const  {return e.data();}
    
  static float dot(const Vec3t& a, const Vec3t& b) {
    return a.e[0]*b.e[0]+a.e[1]*b.e[1]+a.e[2]*b.e[2];
  }
  
  static Vec3t cross(const Vec3t& a, const Vec3t& b) {
    return Vec3t{a.e[1] * b.e[2] - a.e[2] * b.e[1],
                    a.e[2] * b.e[0] - a.e[0] * b.e[2],
                    a.e[0] * b.e[1] - a.e[1] * b.e[0]};
  }
  
  static Vec3t normalize(const Vec3t& a) {
    const float l = a.length();
    return (l != 0.0f) ? a/T(l) : Vec3t{0,0,0};
  }
  
  static Vec3t reflect(const Vec3t& a, const Vec3t& n) {
    return a-n*dot(a,n)*T(2);
  }
  
  static Vec3t refract(const Vec3t& a, const Vec3t& n,
                          const T index) {
    const T cosTheta = T(std::min(-dot(a, n), T(1)));
    const Vec3t rOutParallel{(a + n*cosTheta) * index};
    const Vec3t rOutPerpendicular{n * -sqrt(T(1) - std::min(rOutParallel.sqlength(), T(1)))};
    return rOutParallel + rOutPerpendicular;
  }
      
  static Vec3t minV(const Vec3t& a, const Vec3t& b) {
    return {std::min(a.x,b.x), std::min(a.y,b.y), std::min(a.z,b.z)};
  }
  
  static Vec3t maxV(const Vec3t& a, const Vec3t& b) {
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
};

template <typename T>
std::ostream & operator<<(std::ostream & os, const Vec3t<T> & v) {
   os << v.toString();
   return os;
}

typedef Vec3t<float> Vec3;
typedef Vec3t<int32_t> Vec3i;
typedef Vec3t<uint32_t> Vec3ui;
