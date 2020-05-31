#pragma once 

#include <ostream>
#include <string>
#include <array>

#include "Vec3.h"

class Mat4 {
public:
  Mat4();
  Mat4(float m11, float m12, float m13, float m14,
      float m21, float m22, float m23, float m24,
      float m31, float m32, float m33, float m34,
      float m41, float m42, float m43, float m44);
  Mat4(const std::array<float, 16>& m);
  Mat4(const Mat4& other);
  
  const std::string toString() const;
  friend std::ostream& operator<<(std::ostream &os, const Mat4& m)  {
    os << m.toString() ; return os;
  }


  operator float*(void) {return e.data();}
  operator const float*(void) const  {return e.data();}


  Mat4 operator * ( float scalar ) const;
  Mat4 operator + ( float scalar ) const;
  Mat4 operator - ( float scalar ) const;
  Mat4 operator / ( float scalar ) const;

  Mat4 operator * ( const Mat4& other ) const;
  Vec3 operator * ( const Vec3& other ) const;
  
  static Mat4 scaling(const Vec3& scale);
  static Mat4 translation(const Vec3& trans);
  static Mat4 translation(float x, float y, float z);
  static Mat4 scaling(float x, float y, float z);
  static Mat4 rotationX(float degree);
  static Mat4 rotationY(float degree);
  static Mat4 rotationZ(float degree);
  static Mat4 rotationAxis(const Vec3& axis, float angle);
  static Mat4 transpose(const Mat4& m);
  static Mat4 inverse(const Mat4& m);
  
  static Mat4 perspective(float fovy, float aspect, float znear, float zfar);
  static Mat4 ortho(float left, float right, float bottom, float top, float znear, float zfar );
  static Mat4 lookAt(const Vec3& vEye, const Vec3& vAt, const Vec3& vUp);
  
private:
  std::array<float, 16> e;
  
  static float deg2Rad(const float d);
  
};
