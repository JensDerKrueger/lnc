#pragma once

#include <ostream>
#include <string>
#include <array>
#include <sstream>
#include <cmath>

#include "Vec4.h"
#include "Vec3.h"

template <typename T>
class Mat4t {
public:
  
  struct StereoMatrices {
    Mat4t<T> leftView;
    Mat4t<T> rightView;
    Mat4t<T> leftProj;
    Mat4t<T> rightProj;
  };

  Mat4t() :
  Mat4t{1,0,0,0,
       0,1,0,0,
       0,0,1,0,
       0,0,0,1}
  {
  }
  Mat4t(T m11, T m12, T m13, T m14,
      T m21, T m22, T m23, T m24,
      T m31, T m32, T m33, T m34,
      T m41, T m42, T m43, T m44) :
  e{m11,m12,m13,m14,
    m21,m22,m23,m24,
    m31,m32,m33,m34,
    m41,m42,m43,m44}
  {}
  
  Mat4t(const std::array<T, 16>& e) :
  e{e}
  {
  }
  Mat4t(const Vec3t<T>& e1, T e14,
       const Vec3t<T>& e2, T e24,
       const Vec3t<T>& e3, T e34,
       const Vec3t<T>& e4=Vec3(0,0,0), T e44=1):
  Mat4t(e1.x,e1.y,e1.z,e14,
       e2.x,e2.y,e2.z,e24,
       e3.x,e3.y,e3.z,e34,
       e4.x,e4.y,e4.z,e44)
  {
  }
  Mat4t(const Mat4t& other):
  e{other.e}
  {
  }
  
  const std::string toString() const{
    std::stringstream s;
    s << "[" << e[0] << ", " << e[1] << ", " << e[2] << ", " << e[3] << std::endl <<
         " " << e[4] << ", " << e[5] << ", " << e[6] << ", " << e[7] << std::endl <<
         " " << e[8] << ", " << e[9] << ", " << e[10] << ", " << e[11] << std::endl <<
         " " << e[12] << ", " << e[13] << ", " << e[14] << ", " << e[15] << "]";
    return s.str();
  }
  
  friend std::ostream& operator<<(std::ostream &os, const Mat4t& m)  {
    os << m.toString() ; return os;
  }

  operator T*(void){
    return e.data();
  }
  operator const T*(void) const{
    return e.data();
  }

  // binary operators with scalars
  Mat4t operator * ( T scalar ) const{
    return Mat4t{e[0]*scalar,e[1]*scalar,e[2]*scalar,e[3]*scalar,
                e[4]*scalar,e[5]*scalar,e[6]*scalar,e[7]*scalar,
                e[8]*scalar,e[9]*scalar,e[10]*scalar,e[11]*scalar,
                e[12]*scalar,e[13]*scalar,e[14]*scalar,e[15]*scalar};
  }
  Mat4t operator + ( T scalar ) const{
    return Mat4t{e[0]+scalar,e[1]+scalar,e[2]+scalar,e[3]+scalar,
                e[4]+scalar,e[5]+scalar,e[6]+scalar,e[7]+scalar,
                e[8]+scalar,e[9]+scalar,e[10]+scalar,e[11]+scalar,
                e[12]+scalar,e[13]+scalar,e[14]+scalar,e[15]+scalar};
  }
  
  Mat4t operator - ( T scalar ) const{
    return Mat4t{e[0]-scalar,e[1]-scalar,e[2]-scalar,e[3]-scalar,
                e[4]-scalar,e[5]-scalar,e[6]-scalar,e[7]-scalar,
                e[8]-scalar,e[9]-scalar,e[10]-scalar,e[11]-scalar,
                e[12]-scalar,e[13]-scalar,e[14]-scalar,e[15]-scalar};
  }
  Mat4t operator / ( T scalar ) const {
    return Mat4t{e[0]/scalar,e[1]/scalar,e[2]/scalar,e[3]/scalar,
                e[4]/scalar,e[5]/scalar,e[6]/scalar,e[7]/scalar,
                e[8]/scalar,e[9]/scalar,e[10]/scalar,e[11]/scalar,
                e[12]/scalar,e[13]/scalar,e[14]/scalar,e[15]/scalar};
  }

  Mat4t operator * ( const Mat4t& other ) const{
    Mat4t result;
    for (uint8_t x = 0;x<16;x+=4)
      for (uint8_t y = 0;y<4;y++)
        result.e[x+y] = e[0+x] * other.e[0+y]+
                        e[1+x] * other.e[4+y]+
                        e[2+x] * other.e[8+y]+
                        e[3+x] * other.e[12+y];
    return result;
  }
  Vec3t<T> operator * ( const Vec3t<T>& other ) const{
    T w = other.x*e[12]+other.y*e[13]+other.z*e[14]+1*e[15];
    return {(other.x*e[0]+other.y*e[1]+other.z*e[2]+1*e[3])/w,
                (other.x*e[4]+other.y*e[5]+other.z*e[6]+1*e[7])/w,
                (other.x*e[8]+other.y*e[9]+other.z*e[10]+1*e[11])/w};
  }
  Vec4t<T> operator * ( const Vec4t<T>& other ) const{
    return {(other.x*e[0]+other.y*e[1]+other.z*e[2]+other.w*e[3]),
                (other.x*e[4]+other.y*e[5]+other.z*e[6]+other.w*e[7]),
                (other.x*e[8]+other.y*e[9]+other.z*e[10]+other.w*e[11]),
                (other.x*e[12]+other.y*e[13]+other.z*e[14]+other.w*e[15])};
  }
  
  static Mat4t scaling(T scale) {
    return scaling(scale,scale,scale);
  }
  static Mat4t scaling(const Vec3t<T>& scale){
    return scaling(scale.x, scale.y, scale.z);
  }
  static Mat4t translation(const Vec3t<T>& trans){
    return translation(trans.x, trans.y, trans.z);
  }
  static Mat4t translation(T x, T y, T z){
    return {1, 0, 0, x,
            0, 1, 0, y,
            0, 0, 1, z,
            0, 0, 0, 1};
  }
  static Mat4t scaling(T x, T y, T z){
    return {x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1};
  }
  static Mat4t rotationX(T degree){
    const T angle{deg2Rad(degree)};
    const T cosAngle{T(cos(angle))};
    const T sinAngle{T(sin(angle))};

    return {1, 0, 0, 0,
            0, cosAngle, sinAngle, 0,
            0, -sinAngle, cosAngle, 0,
            0, 0, 0, 1};
  }
  static Mat4t rotationY(T degree){
    const T angle{deg2Rad(degree)};
    const T cosAngle{T(cos(angle))};
    const T sinAngle{T(sin(angle))};

    return {cosAngle, 0, -sinAngle, 0,
            0, 1, 0, 0,
            sinAngle, 0, cosAngle, 0,
            0, 0, 0, 1};
  }
  static Mat4t rotationZ(T degree){
    const T angle{deg2Rad(degree)};
    const T cosAngle{cos(T(angle))};
    const T sinAngle{sin(T(angle))};

    return {cosAngle, sinAngle, 0, 0,
            -sinAngle, cosAngle, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1};
  }
  static Mat4t rotationAxis(const Vec3t<T>& axis, T degree) {
    const T a{deg2Rad(degree)};
    const T cosA{T(cos(a))};
    const T sinA{T(sin(a))};
    const T omCosA{1-cosA};

    const Vec3t<T> sqrAxis{axis * axis};

    return {cosA+omCosA*sqrAxis.x,             omCosA*axis.x*axis.y+sinA*axis.z, omCosA*axis.x*axis.z-sinA*axis.y, 0,
            omCosA*axis.x*axis.y-sinA*axis.z,  cosA+omCosA*sqrAxis.y,                omCosA*axis.y*axis.z+sinA*axis.x, 0,
            omCosA*axis.x*axis.z+sinA*axis.y,  omCosA*axis.y*axis.z-sinA*axis.x, cosA+omCosA*sqrAxis.z,               0,
            0,                                       0,                                      0,                                      1
    };
  }
  static Mat4t transpose(const Mat4t& m){
    return {m.e[0],m.e[4],m.e[8],m.e[12],
            m.e[1],m.e[5],m.e[9],m.e[13],
            m.e[2],m.e[6],m.e[10],m.e[14],
            m.e[3],m.e[7],m.e[11],m.e[15]};
  }
  
  static T det(const Mat4t& m) {
    return   m.e[4] *(m.e[11]*( m.e[1] * m.e[14] - m.e[2]  * m.e[13])+
            m.e[3] *(-m.e[9] * m.e[14] + m.e[13] * m.e[10])+
            m.e[15]*( m.e[2] *  m.e[9] - m.e[1]  * m.e[10]))
          +
          m.e[7] *(m.e[0] *( m.e[9] * m.e[14] - m.e[13] * m.e[10])+
            m.e[2] *(-m.e[12]* m.e[9]  + m.e[8]  * m.e[13])+
            m.e[1] *(-m.e[8] * m.e[14] + m.e[12] * m.e[10]))
          +
          m.e[15]*(m.e[5] *(-m.e[8] *  m.e[2] + m.e[0]  * m.e[10])+
            m.e[6] *(-m.e[0] *  m.e[9] + m.e[1]  * m.e[8]))
          +
          m.e[11]*(m.e[0] *(-m.e[5] * m.e[14] + m.e[6]  * m.e[13])+
            m.e[12]*( m.e[2] * m.e[5]  - m.e[6]  * m.e[1]))
          +
          m.e[3]* (m.e[6] *( m.e[9] * m.e[12] - m.e[13] * m.e[8])+
            m.e[5] *( m.e[8] * m.e[14] - m.e[12] * m.e[10]));
 }

  static Mat4t inverse(const Mat4t& m) {
    return Mat4t::inverse(m, Mat4t::det(m));
  }
  
  static Mat4t inverse(const Mat4t& m, T det) {
    T Q = T(1.0/det);

    Mat4t result;
   
    result.e[0] =  ( m.e[7]  * m.e[9]  * m.e[14]
        + m.e[15] * m.e[5]  * m.e[10]
        - m.e[15] * m.e[6]  * m.e[9]
        - m.e[11] * m.e[5]  * m.e[14]
        - m.e[7]  * m.e[13] * m.e[10]
        + m.e[11] * m.e[6]  * m.e[13])*Q;
      result.e[4] = -( m.e[4]  * m.e[15] * m.e[10]
        - m.e[4]  * m.e[11] * m.e[14]
        - m.e[15] * m.e[6]  * m.e[8]
        + m.e[11] * m.e[6]  * m.e[12]
        + m.e[7]  * m.e[8]  * m.e[14]
        - m.e[7]  * m.e[12] * m.e[10])*Q;
    result.e[8] = (- m.e[4]  * m.e[11] * m.e[13]
        + m.e[4]  * m.e[15] * m.e[9]
        - m.e[15] * m.e[8]  * m.e[5]
        - m.e[7]  * m.e[12] * m.e[9]
        + m.e[11] * m.e[12] * m.e[5]
        + m.e[7]  * m.e[8]  * m.e[13])*Q;
      result.e[12] =  -(m.e[4]  * m.e[9]  * m.e[14]
        - m.e[4]  * m.e[13] * m.e[10]
        + m.e[12] * m.e[5]  * m.e[10]
        - m.e[9]  * m.e[6]  * m.e[12]
        - m.e[8]  * m.e[5]  * m.e[14]
        + m.e[13] * m.e[6]  * m.e[8])*Q;
    /// 2
    result.e[1] = (- m.e[1]  * m.e[15] * m.e[10]
        + m.e[1]  * m.e[11] * m.e[14]
        - m.e[11] * m.e[2]  * m.e[13]
        - m.e[3]  * m.e[9]  * m.e[14]
        + m.e[15] * m.e[2]  * m.e[9]
        + m.e[3]  * m.e[13] * m.e[10])*Q;

    result.e[5] = (- m.e[15] * m.e[2]  * m.e[8]
        + m.e[15] * m.e[0]  * m.e[10]
        - m.e[11] * m.e[0]  * m.e[14]
        - m.e[3]  * m.e[12] * m.e[10]
        + m.e[11] * m.e[2]  * m.e[12]
        + m.e[3]  * m.e[8]  * m.e[14])*Q;

    result.e[9] = -(-m.e[1]  * m.e[15] * m.e[8]
        + m.e[1]  * m.e[11] * m.e[12]
        + m.e[15] * m.e[0]  * m.e[9]
        - m.e[3]  * m.e[9]  * m.e[12]
        + m.e[3]  * m.e[13] * m.e[8]
        - m.e[11] * m.e[0]  * m.e[13])*Q;

    result.e[13] = (- m.e[1]  * m.e[8]  * m.e[14]
        + m.e[1]  * m.e[12] * m.e[10]
        + m.e[0]  * m.e[9]  * m.e[14]
        - m.e[0]  * m.e[13] * m.e[10]
        - m.e[12] * m.e[2]  * m.e[9]
        + m.e[8]  * m.e[2]  * m.e[13])*Q;
    /// 3
    result.e[2] = -( m.e[15] * m.e[2]  * m.e[5]
        - m.e[7]  * m.e[2]  * m.e[13]
        - m.e[3]  * m.e[5]  * m.e[14]
        + m.e[1]  * m.e[7]  * m.e[14]
        - m.e[1]  * m.e[15] * m.e[6]
        + m.e[3]  * m.e[13] * m.e[6])*Q;

    result.e[6] = (- m.e[4]  * m.e[3]  * m.e[14]
        + m.e[4]  * m.e[15] * m.e[2]
        + m.e[7]  * m.e[0]  * m.e[14]
        - m.e[15] * m.e[6]  * m.e[0]
        - m.e[7]  * m.e[12] * m.e[2]
        + m.e[3]  * m.e[6]  * m.e[12])*Q;

    result.e[10] = -(-m.e[15] * m.e[0]  * m.e[5]
        + m.e[15] * m.e[1]  * m.e[4]
        + m.e[3]  * m.e[12] * m.e[5]
        + m.e[7]  * m.e[0]  * m.e[13]
        - m.e[7]  * m.e[1]  * m.e[12]
        - m.e[3]  * m.e[4]  * m.e[13])*Q;

    result.e[14] = -( m.e[14] * m.e[0]  * m.e[5]
        - m.e[14] * m.e[1]  * m.e[4]
        - m.e[2]  * m.e[12] * m.e[5]
        - m.e[6]  * m.e[0]  * m.e[13]
        + m.e[6]  * m.e[1]  * m.e[12]
        + m.e[2]  * m.e[4]  * m.e[13])*Q;
    /// 4
    result.e[3] = (- m.e[1]  * m.e[11] * m.e[6]
        + m.e[1]  * m.e[7]  * m.e[10]
        - m.e[7]  * m.e[2]  * m.e[9]
        - m.e[3]  * m.e[5]  * m.e[10]
        + m.e[11] * m.e[2]  * m.e[5]
        + m.e[3]  * m.e[9]  * m.e[6])*Q;

    result.e[7] = -(-m.e[4]  * m.e[3]  * m.e[10]
        + m.e[4]  * m.e[11] * m.e[2]
        + m.e[7]  * m.e[0]  * m.e[10]
        - m.e[11] * m.e[6]  * m.e[0]
        + m.e[3]  * m.e[6]  * m.e[8]
        - m.e[7]  * m.e[8]  * m.e[2])*Q;

    result.e[11] = (- m.e[11] * m.e[0]  * m.e[5]
        + m.e[11] * m.e[1]  * m.e[4]
        + m.e[3]  * m.e[8]  * m.e[5]
        + m.e[7]  * m.e[0]  * m.e[9]
        - m.e[7]  * m.e[1]  * m.e[8]
        - m.e[3]  * m.e[4]  * m.e[9])*Q;

    result.e[15] =  ( m.e[10] * m.e[0]  * m.e[5]
        - m.e[10] * m.e[1]  * m.e[4]
        - m.e[2]  * m.e[8]  * m.e[5]
        - m.e[6]  * m.e[0]  * m.e[9]
        + m.e[6]  * m.e[1]  * m.e[8]
        + m.e[2]  * m.e[4]  * m.e[9])*Q;
    return result;
  }
  
  static Mat4t perspective(T fovy, T aspect, T znear, T zfar){
    T cotan {T(1.0/tan(Mat4t::deg2Rad(fovy)/2.0))};

    return {cotan/aspect, 0, 0, 0,
            0, cotan, 0, 0,
            0, 0, -(zfar+znear)/(zfar-znear), -2*(zfar*znear)/(zfar-znear),
            0, 0, -1, 0};
  }
  static Mat4t perspective(T left, T right, T bottom, T top, T znear, T zfar){
    return {2*znear/(right-left),   0,                      (right+left)/(right-left),       0,
            0,                      2*znear/(top-bottom),   (top+bottom)/(top-bottom),       0,
            0,                      0,                      -(zfar+znear)/(zfar-znear),     -2*(zfar*znear)/(zfar-znear),
            0,                      0,                      -1,                           0};

  }
  static Mat4t ortho(T left, T right, T bottom, T top, T znear, T zfar ){
    return {2/(right-left), 0, 0, -(right+left)/(right-left),
            0, 2/(top-bottom), 0, -(top+bottom)/(top-bottom),
            0, 0, -2/(zfar-znear), -(zfar+znear)/(zfar-znear),
            0, 0, 0, 1};
  }
  static Mat4t lookAt(const Vec3t<T>& vEye, const Vec3t<T>& vAt, const Vec3t<T>& vUp){
    Vec3t<T> f{vAt-vEye};
    Vec3t<T> u{vUp};
    Vec3t<T> s{Vec3::cross(f,u)};
    u = Vec3t<T>::cross(s,f);

    f = Vec3t<T>::normalize(f);
    u = Vec3t<T>::normalize(u);
    s = Vec3t<T>::normalize(s);

    return {s.x, s.y, s.z, -Vec3t<T>::dot(s,vEye),
            u.x, u.y, u.z, -Vec3t<T>::dot(u,vEye),
            -f.x, -f.y, -f.z, Vec3t<T>::dot(f,vEye),
            0,0,0,1.0};
  }
  static Mat4t mirror(const Vec3t<T>& p, const Vec3t<T>& n) {
    T k = Vec3t<T>::dot(p,n);

    return {1-2*n.x*n.x,-2*n.x*n.y,-2*n.x*n.z,2*k*n.x,
      -2*n.y*n.x,1-2*n.y*n.y,-2*n.y*n.z,2*k*n.y,
      -2*n.z*n.x,-2*n.z*n.y,1-2*n.z*n.z,2*k*n.z,
      0,0,0,1
    };
  }
  
  static StereoMatrices stereoLookAtAndProjection(const Vec3t<T>& eye, const Vec3t<T>& at, const Vec3t<T>& up,
                                           T fovy, T aspect, T znear, T zfar, T focalLength,
                                           T eyeDist) {
    
    StereoMatrices result;
    T wd2     = znear * tanf(Mat4t::deg2Rad(fovy)/2);
    T nfdl    = znear / focalLength;
    T shift   =   eyeDist * nfdl;
    T top     =   wd2;
    T bottom  = - wd2;

    // projection matrices
    T left    = - aspect * wd2 - shift;
    T right   =   aspect * wd2 - shift;
    result.leftProj = Mat4t::perspective(left, right, bottom, top, znear, zfar);
    left    = - aspect * wd2 + shift;
    right   =   aspect * wd2 + shift;
    result.rightProj = Mat4t::perspective(left, right, bottom, top, znear, zfar);
    
    // view matrices
    result.leftView  = Mat4t::translation(-eyeDist/2, 0, 0) * Mat4t::lookAt(eye, at, up);
    result.rightView = Mat4t::translation(eyeDist/2, 0, 0) * Mat4t::lookAt(eye, at, up);
    
    return result;
  }

private:
  std::array<T, 16> e;
  
  static T deg2Rad(const T d) {
    return T(3.14159265358979323846)*d/T(180);
  }
};

typedef Mat4t<float> Mat4;
