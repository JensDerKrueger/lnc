#include <sstream>
#include <cmath>
#include "Mat3.h"

#ifndef M_PI
constexpr float M_PI = 3.14159265358979323846f;
#endif

Mat3::Mat3() :
	Mat3{1,0,0,
	     0,1,0,
	     0,0,1}
{
}

Mat3::Mat3(float m11, float m12, float m13,
           float m21, float m22, float m23,
           float m31, float m32, float m33) :
	e{m11,m12,m13,
	  m21,m22,m23,
	  m31,m32,m33}
{		
}

Mat3::Mat3(const std::array<float, 9>& e) :
	e{e}
{
}

Mat3::Mat3(const Mat3& other) :
	e{other.e}
{	
}

Mat3::Mat3(const Vec3& e1,const Vec3& e2, const Vec3& e3) :
	Mat3(e1.x(),e1.y(),e1.z(),
		   e2.x(),e2.y(),e2.z(),
		   e3.x(),e3.y(),e3.z())
{
}

const std::string Mat3::toString() const {
	std::stringstream s;
	s << "[" << e[0] << ", " << e[1] << ", " << e[2] << std::endl <<
       " " << e[3] << ", " << e[4] << ", " << e[5] << std::endl <<
		   " " << e[6] << ", " << e[7] << ", " << e[8] <<  "]";
	return s.str();
}

Mat3::operator float*(void) {
  return e.data();
}

Mat3::operator const float*(void) const  {
  return e.data();
}

// binary operators with scalars
Mat3 Mat3::operator* ( float scalar ) const {
	return Mat3{e[0]*scalar,e[1]*scalar,e[2]*scalar,
              e[3]*scalar,e[4]*scalar,e[5]*scalar,
              e[6]*scalar,e[7]*scalar,e[8]*scalar};
}

Mat3 Mat3::operator+ ( float scalar ) const {
	return Mat3{e[0]+scalar,e[1]+scalar,e[2]+scalar,e[3]+scalar,
              e[4]+scalar,e[5]+scalar,e[6]+scalar,e[7]+scalar,
              e[8]+scalar};
}

Mat3 Mat3::operator- ( float scalar ) const {
	return Mat3{e[0]-scalar,e[1]-scalar,e[2]-scalar,e[3]-scalar,
              e[4]-scalar,e[5]-scalar,e[6]-scalar,e[7]-scalar,
              e[8]-scalar};
}

Mat3 Mat3::operator/ ( float scalar ) const {
	return Mat3{e[0]/scalar,e[1]/scalar,e[2]/scalar,e[3]/scalar,
              e[4]/scalar,e[5]/scalar,e[6]/scalar,e[7]/scalar,
              e[8]/scalar};
}

Mat3 Mat3::operator * ( const Mat3& other ) const {
	Mat3 result;
	for (int x = 0;x<9;x+=3)
		for (int y = 0;y<3;y++)
			result.e[x+y] = other.e[0+x] * e[0+y]+
                      other.e[1+x] * e[3+y]+
                      other.e[2+x] * e[6+y];
	return result;
}

Vec3 Mat3::operator * ( const Vec3& other ) const {
	return Vec3{(other.x()*e[0]+other.y()*e[1]+other.z()*e[2]),
              (other.x()*e[3]+other.y()*e[4]+other.z()*e[5]),
              (other.x()*e[6]+other.y()*e[7]+other.z()*e[8])};
}


Mat3 Mat3::scaling(const Vec3& scale) {
	return scaling(scale.x(), scale.y(), scale.z());
}

Mat3 Mat3::scaling(float x, float y, float z) {
	return {x, 0, 0,
          0, y, 0,
          0, 0, z};
}

float Mat3::deg2Rad(const float d) {
    return M_PI*d/180.0f;
}


Mat3 Mat3::rotationX(float degree) {
	const float angle{deg2Rad(degree)};
	const float cosAngle = cosf(angle);
	const float sinAngle = sinf(angle);

	return {1, 0, 0,
          0, cosAngle, sinAngle,
          0, -sinAngle, cosAngle};
}


Mat3 Mat3::rotationY(float degree) {
	const float angle{deg2Rad(degree)};
	const float cosAngle{cosf(angle)};
	const float sinAngle{sinf(angle)};

	return {cosAngle, 0, -sinAngle,
          0, 1, 0,
          sinAngle, 0, cosAngle};
}


Mat3 Mat3::rotationZ(float degree) {
	const float angle{deg2Rad(degree)};
	const float cosAngle{cosf(angle)};
	const float sinAngle{sinf(angle)};

	return {cosAngle, sinAngle, 0,
          -sinAngle, cosAngle, 0,
          0, 0, 1};
}

Mat3 Mat3::transpose(const Mat3& m) {
	return {m.e[0],m.e[3],m.e[6],
          m.e[1],m.e[4],m.e[7],
          m.e[2],m.e[5],m.e[8]};
}

float Mat3::det(const Mat3& m) {
  return m.e[0]*(m.e[4]*m.e[8]-m.e[5]*m.e[7])-m.e[1]*(m.e[3]*m.e[8]-m.e[5]*m.e[6])+m.e[2]*(m.e[3]*m.e[7]-m.e[4]*m.e[6]);
}

Mat3 Mat3::inverse(const Mat3& m, float det) {
  float Q = 1.0f/det;

  return {
    (m.e[4]*m.e[8]-m.e[5]*m.e[7])*Q, (m.e[2]*m.e[7]-m.e[1]*m.e[8])*Q, (m.e[1]*m.e[5]-m.e[2]*m.e[4])*Q,
    (m.e[5]*m.e[6]-m.e[3]*m.e[8])*Q, (m.e[0]*m.e[8]-m.e[2]*m.e[6])*Q, (m.e[2]*m.e[3]-m.e[0]*m.e[5])*Q,
    (m.e[3]*m.e[7]-m.e[4]*m.e[6])*Q, (m.e[1]*m.e[6]-m.e[0]*m.e[7])*Q, (m.e[0]*m.e[4]-m.e[1]*m.e[3])*Q
  };
}

Mat3 Mat3::inverse(const Mat3& m) {
  return Mat3::inverse(m, Mat3::det(m));
}
