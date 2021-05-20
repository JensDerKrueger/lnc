#pragma once 

#include <ostream>
#include <string>
#include <array>
#include <sstream>
#include <cmath>

#include "Rand.h"

template <typename T>
union Vec2t {
public:
  std::array<T, 2> e;
  struct { T x; T y; };
  struct { T r; T g; };

  Vec2t() :
    e{0,0}
  {}

  Vec2t(T x, T y) :
    e{x,y}
  {}
      
  Vec2t(const Vec2t& other) :
    e{other.x, other.y}
  {}

  template <typename U>
  explicit Vec2t(const Vec2t<U>& other) :
    e{T(other.x), T(other.y)}
  {}


  const std::string toString() const {
    std::stringstream s;
    s << "[" << e[0] << ", " << e[1] << "]";
    return s.str();
  }

  Vec2t operator+(const Vec2t& val) const{
    return {e[0]+val.e[0],e[1]+val.e[1]};
  }

  Vec2t operator-(const Vec2t& val) const {
    return {e[0]-val.e[0],e[1]-val.e[1]};
  }

  Vec2t operator*(const Vec2t& val) const {
    return {e[0]*val.e[0],e[1]*val.e[1]};
  }

  Vec2t operator/(const Vec2t& val) const {
    return {e[0]/val.e[0],e[1]/val.e[1]};
  }

  Vec2t operator*(const T& val) const {
    return {e[0]*val,e[1]*val};
  }

  Vec2t operator/(const T& val) const {
    return {e[0]/val, e[1]/val};
  }

  bool operator == ( const Vec2t& other ) const {
      return e == other.e;
  }

  bool operator != ( const Vec2t& other ) const {
      return e != other.e;
  }
    
  T length() const {
    return sqrt(sqlength());
  }
    
  T sqlength() const {
    return e[0]*e[0]+e[1]*e[1];
  }

  friend std::ostream& operator<<(std::ostream &os, const Vec2t& v) {os << v.toString() ; return os;}

  operator T*(void) {return e.data();}
  operator const T*(void) const  {return e.data();}
          
  static Vec2t random() {
      return Vec2t{T{Rand::rand01()},T{Rand::rand01()}};
  }
  
  static Vec2t normalize(const Vec2t& a) {
    const float l = a.length();
    return (l != T(0)) ? a/l : Vec2t{T(0),T(0)};
  }
		
};

typedef Vec2t<float> Vec2;
typedef Vec2t<int32_t> Vec2i;
typedef Vec2t<uint32_t> Vec2ui;
