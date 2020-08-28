#pragma once 

#include <ostream>
#include <string>
#include <array>
#include <sstream>
#include <cmath>

#include "Rand.h"


template <typename T>
class Vec2t {
public:
  Vec2t() :
    e{0,0}
  {}

  Vec2t(T x, T y) :
    e{x,y}
  {}
      
  template <typename U>
  Vec2t(const Vec2t<U>& other) :
    e{T(other.x()), T(other.y())}
  {}

  const std::string toString() const {
    std::stringstream s;
    s << "[" << e[0] << ", " << e[1] << "]";
    return s.str();
  }

  T x() const {
    return e[0];
  }

  T y() const {
    return e[1];
  }

  Vec2t<T> operator+(const Vec2t<T>& val) const{
    return {e[0]+val.e[0],e[1]+val.e[1]};
  }

  Vec2t<T> operator-(const Vec2t<T>& val) const {
    return {e[0]-val.e[0],e[1]-val.e[1]};
  }

  Vec2t<T> operator*(const Vec2t<T>& val) const {
    return {e[0]*val.e[0],e[1]*val.e[1]};
  }

  Vec2t<T> operator/(const Vec2t<T>& val) const {
    return {e[0]/val.e[0],e[1]/val.e[1]};
  }

  Vec2t<T> operator*(const T& val) const {
    return {e[0]*val,e[1]*val};
  }

  Vec2t<T> operator/(const T& val) const {
    return {e[0]/val, e[1]/val};
  }

  bool operator == ( const Vec2t<T>& other ) const {
      return e == other.e;
  }

  bool operator != ( const Vec2t<T>& other ) const {
      return e != other.e;
  }
    
  float length() const {
    return sqrt(sqlength());
  }
    
  float sqlength() const {
    return e[0]*e[0]+e[1]*e[1];
  }

  friend std::ostream& operator<<(std::ostream &os, const Vec2t<T>& v) {os << v.toString() ; return os;}

  operator T*(void) {return e.data();}
  operator const T*(void) const  {return e.data();}
          
  static Vec2t<T> random() {
      return Vec2t<T>{T{Rand::rand01()},T{Rand::rand01()}};
  }
  
private:
  std::array<T, 2> e;
		
};

typedef Vec2t<float> Vec2;
typedef Vec2t<int32_t> Vec2i;
typedef Vec2t<uint32_t> Vec2ui;
