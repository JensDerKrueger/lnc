#pragma once 

#include <ostream>
#include <string>
#include <array>

#include "Vec3.h"

struct StereoMatrices;

class Mat3 {
public:
  Mat3();
  Mat3(float m11, float m12, float m13,
      float m21, float m22, float m23,
      float m31, float m32, float m33);
  Mat3(const std::array<float, 9>& m);
  Mat3(const Vec3& e1,
       const Vec3& e2,
       const Vec3& e3);
  Mat3(const Mat3& other);
  
  const std::string toString() const;
  friend std::ostream& operator<<(std::ostream &os, const Mat3& m)  {
    os << m.toString() ; return os;
  }


  operator float*(void);
  operator const float*(void) const;

  Mat3 operator * ( float scalar ) const;
  Mat3 operator + ( float scalar ) const;
  Mat3 operator - ( float scalar ) const;
  Mat3 operator / ( float scalar ) const;

  Mat3 operator * ( const Mat3& other ) const;
  Vec3 operator * ( const Vec3& other ) const;
  
  static Mat3 scaling(const Vec3& scale);
  static Mat3 scaling(float x, float y, float z);
  static Mat3 rotationX(float degree);
  static Mat3 rotationY(float degree);
  static Mat3 rotationZ(float degree);
  static Mat3 transpose(const Mat3& m);
  
  static float det(const Mat3& m);
  static Mat3 inverse(const Mat3& m, float det);
  static Mat3 inverse(const Mat3& m);

private:
  std::array<float, 9> e;
  
  static float deg2Rad(const float d);
  
};
