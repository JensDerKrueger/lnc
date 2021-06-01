#pragma once 

#include <ostream>
#include <string>
#include <array>

#include "Vec2.h"
#include "Vec3.h"

template <typename T>
union Vec4t {
public:
  std::array<T, 4> e;
  struct { T x; T y; T z; T w;};
  struct { T r; T g; T b; T a;};
  Vec2t<T> xy;
  Vec3t<T> xyz;

  Vec4t():
    e{0,0,0,0}
  {}
  
  Vec4t(T x, T y, T z, T w):
    e{x,y,z,w}
  {}
  
  Vec4t(const Vec4t& other) :
    e{other.x, other.y, other.z, other.w}
  {}
  
  Vec4t(const Vec3t<T>& other, T w):
    e{other.x, other.y, other.z, w}
  {}
  
  Vec4t(const Vec2t<T>& other, T z, T w) :
    e{other.x, other.y, z, w}
  {}
  
  template <typename U>
  explicit Vec4t(const Vec4t<U>& other) :
    e{other.x, other.y, other.z, other.w}
  {}
  
  template <typename U>
  explicit Vec4t(const Vec3t<U>& other, T w):
    e{other.x, other.y, other.z}
  {}
  
  template <typename U>
  explicit Vec4t(const Vec2t<U>& other, T z, T w) :
    e{other.x, other.y, z, w}
  {}

  friend std::ostream& operator<<(std::ostream &os, const Vec4t& v) {os << v.toString() ; return os;}
  const std::string toString() const {
    std::stringstream s;
    s << "[" << e[0] << ", " << e[1] << ", " << e[2] << ", " << e[3] << "]";
    return s.str();
  }
  
  Vec4t operator+(const Vec4t& val) const {
    return {e[0]+val.e[0],
            e[1]+val.e[1],
            e[2]+val.e[2],
            e[3]+val.e[3]};
  }
  
  Vec4t operator-(const Vec4t& val) const {
    return {e[0]-val.e[0],
            e[1]-val.e[1],
            e[2]-val.e[2],
            e[3]-val.e[3]};
  }
  
  Vec4t operator*(const Vec4t& val) const {
    return {e[0]*val.e[0],
            e[1]*val.e[1],
            e[2]*val.e[2],
            e[3]*val.e[3]};
  }
  
  Vec4t operator/(const Vec4t& val) const {
    return {e[0]/val.e[0],
            e[1]/val.e[1],
            e[2]/val.e[2],
            e[3]/val.e[3]};
  }

  Vec4t operator*(const T& val) const {
    return {e[0]*val, e[1]*val, e[2]*val, e[3]*val};
  }
  
  Vec4t operator/(const T& val) const {
    return {e[0]/val, e[1]/val, e[2]/val, e[3]/val};
  }
  
  Vec4t operator+(const T& val) const {
    return {e[0]+val,e[1]+val,e[2]+val,e[3]+val};
  }
  
  Vec4t operator-(const T& val) const{
    return {e[0]-val,e[1]-val,e[2]-val,e[3]-val};
  }
  
  bool operator == ( const Vec4t& other ) const {
    return e[0] == other.e[0] &&
           e[1] == other.e[1] &&
           e[2] == other.e[2] &&
           e[3] == other.e[3];
  }
  
  bool operator != ( const Vec4t& other ) const {
    return e[0] != other.e[0] ||
           e[1] != other.e[1] ||
           e[2] != other.e[2] ||
           e[3] != other.e[3];
  }

  T length() const {
    return sqrt(sqlength());
  }
  
  T sqlength() const {
    return e[0]*e[0]+e[1]*e[1]+e[2]*e[2]+e[3]*e[3];
  }
  
  Vec3t<T> vec3() const {
    return {e[0], e[1], e[2]};
  }
  
  operator T*(void) {return e.data();}
  operator const T*(void) const  {return e.data();}
    
  static T dot(const Vec4t& a, const Vec4t& b) {
    return a.e[0]*b.e[0]+a.e[1]*b.e[1]+a.e[2]*b.e[2]+a.e[3]*b.e[3];
  }
  
  static Vec4t normalize(const Vec4t& a) {
    return a/a.length();
  }
      
  static Vec4t<float> random() {
    return {Rand::rand01(),Rand::rand01(),Rand::rand01(),Rand::rand01()};
  }
};

typedef Vec4t<float> Vec4;
typedef Vec4t<int32_t> Vec4i;
typedef Vec4t<uint32_t> Vec4ui;
