#pragma once 

#include <ostream>
#include <string>
#include <array>
#include <sstream>
#include <cmath>

#include "Vec3.h"

template <typename T>
class Mat3t {
public:
  Mat3t():
  Mat3t{1,0,0,
       0,1,0,
       0,0,1}
  {}
  Mat3t(T m11, T m12, T m13,
      T m21, T m22, T m23,
      T m31, T m32, T m33):
  e{m11,m12,m13,
    m21,m22,m23,
    m31,m32,m33}
  {
  }
  Mat3t(const std::array<T, 9>& e):
  e{e}
  {
  }
  Mat3t(const Vec3t<T>& e1,
       const Vec3t<T>& e2,
       const Vec3t<T>& e3):
  Mat3t(e1.x,e1.y,e1.z,
       e2.x,e2.y,e2.z,
       e3.x,e3.y,e3.z)
  {
  }
  Mat3t(const Mat3t& other):
  e{other.e}
  {
  }
  
  const std::string toString() const{
    std::stringstream s;
    s << "[" << e[0] << ", " << e[1] << ", " << e[2] << std::endl <<
         " " << e[3] << ", " << e[4] << ", " << e[5] << std::endl <<
         " " << e[6] << ", " << e[7] << ", " << e[8] <<  "]";
    return s.str();
  }
  
  friend std::ostream& operator<<(std::ostream &os, const Mat3t& m)  {
    os << m.toString() ; return os;
  }

  operator T*(void){
    return e.data();
  }
  
  operator const T*(void) const{
    return e.data();
  }

  Mat3t operator * ( T scalar ) const{
    return Mat3t{e[0]*scalar,e[1]*scalar,e[2]*scalar,
                e[3]*scalar,e[4]*scalar,e[5]*scalar,
                e[6]*scalar,e[7]*scalar,e[8]*scalar};
  }
  Mat3t operator + ( T scalar ) const{
    return Mat3t{e[0]+scalar,e[1]+scalar,e[2]+scalar,e[3]+scalar,
                e[4]+scalar,e[5]+scalar,e[6]+scalar,e[7]+scalar,
                e[8]+scalar};
  }
  Mat3t operator - ( T scalar ) const{
    return Mat3t{e[0]-scalar,e[1]-scalar,e[2]-scalar,e[3]-scalar,
                e[4]-scalar,e[5]-scalar,e[6]-scalar,e[7]-scalar,
                e[8]-scalar};
  }
  Mat3t operator / ( T scalar ) const{
    return Mat3t{e[0]/scalar,e[1]/scalar,e[2]/scalar,e[3]/scalar,
                e[4]/scalar,e[5]/scalar,e[6]/scalar,e[7]/scalar,
                e[8]/scalar};
  }

  Mat3t operator * ( const Mat3t& other ) const{
    Mat3t result;
    for (uint8_t x = 0;x<9;x+=3)
      for (uint8_t y = 0;y<3;y++)
        result.e[x+y] = e[0+x] * other.e[0+y]+
                        e[1+x] * other.e[3+y]+
                        e[2+x] * other.e[6+y];
    return result;
  }

  Vec3t<T> operator * ( const Vec3t<T>& other ) const{
    return Vec3t<T>{(other.x*e[0]+other.y*e[1]+other.z*e[2]),
                (other.x*e[3]+other.y*e[4]+other.z*e[5]),
                (other.x*e[6]+other.y*e[7]+other.z*e[8])};
  }
  
  static Mat3t scaling(const Vec3t<T>& scale){
    return scaling(scale.x, scale.y, scale.z);
  }
  
  static Mat3t scaling(T x, T y, T z){
    return {x, 0, 0,
            0, y, 0,
            0, 0, z};
  }
  
  static Mat3t rotationX(T degree){
    const T angle{deg2Rad(degree)};
    const T cosAngle = cosf(angle);
    const T sinAngle = sinf(angle);

    return {1, 0, 0,
            0, cosAngle, sinAngle,
            0, -sinAngle, cosAngle};
  }
  
  static Mat3t rotationY(T degree){
    const T angle{deg2Rad(degree)};
    const T cosAngle{cosf(angle)};
    const T sinAngle{sinf(angle)};

    return {cosAngle, 0, -sinAngle,
            0, 1, 0,
            sinAngle, 0, cosAngle};
  }
  
  static Mat3t rotationZ(T degree){
    const T angle{deg2Rad(degree)};
    const T cosAngle{cosf(angle)};
    const T sinAngle{sinf(angle)};

    return {cosAngle, sinAngle, 0,
            -sinAngle, cosAngle, 0,
            0, 0, 1};
  }
  
  static Mat3t transpose(const Mat3t& m){
    return {m.e[0],m.e[3],m.e[6],
            m.e[1],m.e[4],m.e[7],
            m.e[2],m.e[5],m.e[8]};
  }

  static T det(const Mat3t& m){
    return m.e[0]*(m.e[4]*m.e[8]-m.e[5]*m.e[7])-m.e[1]*(m.e[3]*m.e[8]-m.e[5]*m.e[6])+m.e[2]*(m.e[3]*m.e[7]-m.e[4]*m.e[6]);
  }
  
  static Mat3t inverse(const Mat3t& m, T det) {
    T Q = T(1.0/det);

    return {
      (m.e[4]*m.e[8]-m.e[5]*m.e[7])*Q, (m.e[2]*m.e[7]-m.e[1]*m.e[8])*Q, (m.e[1]*m.e[5]-m.e[2]*m.e[4])*Q,
      (m.e[5]*m.e[6]-m.e[3]*m.e[8])*Q, (m.e[0]*m.e[8]-m.e[2]*m.e[6])*Q, (m.e[2]*m.e[3]-m.e[0]*m.e[5])*Q,
      (m.e[3]*m.e[7]-m.e[4]*m.e[6])*Q, (m.e[1]*m.e[6]-m.e[0]*m.e[7])*Q, (m.e[0]*m.e[4]-m.e[1]*m.e[3])*Q
    };
  }

  static Mat3t inverse(const Mat3t& m) {
    return Mat3t::inverse(m, Mat3t::det(m));
  }


private:
  std::array<T, 9> e;
  
  static T deg2Rad(const T d) {
    return T(3.14159265358979323846)*d/T(180);
  }
};

typedef Mat3t<float> Mat3;
