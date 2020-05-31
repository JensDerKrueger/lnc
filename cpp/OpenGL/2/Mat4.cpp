#include <sstream>
#include <cmath>
#include "Mat4.h"

#ifndef M_PI
constexpr float M_PI = 3.14159265358979323846f;
#endif

Mat4::Mat4() :
	Mat4{1,0,0,0,
	     0,1,0,0,
	     0,0,1,0,
	     0,0,0,1}
{
}

Mat4::Mat4(float m11, float m12, float m13, float m14,
		float m21, float m22, float m23, float m24,
		float m31, float m32, float m33, float m34,
		float m41, float m42, float m43, float m44) :
	e{m11,m12,m13,m14,
	  m21,m22,m23,m24,
	  m31,m32,m33,m34,
	  m41,m42,m43,m44}
{		
}

Mat4::Mat4(const std::array<float, 16>& e) :
	e{e}
{
}

Mat4::Mat4(const Mat4& other) :
	e{other.e}
{	
}

const std::string Mat4::toString() const {
	std::stringstream s;
	s << "[" << e[0] << ", " << e[1] << ", " << e[2] << ", " << e[3] << std::endl <<
		 " " << e[4] << ", " << e[5] << ", " << e[6] << ", " << e[7] << std::endl <<
		 " " << e[8] << ", " << e[9] << ", " << e[10] << ", " << e[11] << std::endl <<
		 " " << e[12] << ", " << e[13] << ", " << e[14] << ", " << e[15] << "]";
	return s.str();
}


// binary operators with scalars
Mat4 Mat4::operator* ( float scalar ) const {
	return Mat4{e[0]*scalar,e[1]*scalar,e[2]*scalar,e[3]*scalar,
				e[4]*scalar,e[5]*scalar,e[6]*scalar,e[7]*scalar,
				e[8]*scalar,e[9]*scalar,e[10]*scalar,e[11]*scalar,
				e[12]*scalar,e[13]*scalar,e[14]*scalar,e[15]*scalar};
}

Mat4 Mat4::operator+ ( float scalar ) const {
	return Mat4{e[0]+scalar,e[1]+scalar,e[2]+scalar,e[3]+scalar,
				e[4]+scalar,e[5]+scalar,e[6]+scalar,e[7]+scalar,
				e[8]+scalar,e[9]+scalar,e[10]+scalar,e[11]+scalar,
				e[12]+scalar,e[13]+scalar,e[14]+scalar,e[15]+scalar};
}

Mat4 Mat4::operator- ( float scalar ) const {
	return Mat4{e[0]-scalar,e[1]-scalar,e[2]-scalar,e[3]-scalar,
				e[4]-scalar,e[5]-scalar,e[6]-scalar,e[7]-scalar,
				e[8]-scalar,e[9]-scalar,e[10]-scalar,e[11]-scalar,
				e[12]-scalar,e[13]-scalar,e[14]-scalar,e[15]-scalar};
}

Mat4 Mat4::operator/ ( float scalar ) const {
	return Mat4{e[0]/scalar,e[1]/scalar,e[2]/scalar,e[3]/scalar,
				e[4]/scalar,e[5]/scalar,e[6]/scalar,e[7]/scalar,
				e[8]/scalar,e[9]/scalar,e[10]/scalar,e[11]/scalar,
				e[12]/scalar,e[13]/scalar,e[14]/scalar,e[15]/scalar};
}

Mat4 Mat4::operator * ( const Mat4& other ) const {
	Mat4 result;
	for (int x = 0;x<16;x+=4)
		for (int y = 0;y<4;y++)
			result.e[x+y] = e[0+x] * other.e[0+y]+
							e[1+x] * other.e[4+y]+
							e[2+x] * other.e[8+y]+
							e[3+x] * other.e[12+y];
	return result;
}

Vec3 Mat4::operator * ( const Vec3& other ) const {
	
	float w = other.x()*e[3]+other.y()*e[7]+other.z()*e[11]+1*e[15];
	
	return Vec3{(other.x()*e[0]+other.y()*e[4]+other.z()*e[8]+1*e[12])/w,
				(other.x()*e[1]+other.y()*e[5]+other.z()*e[9]+1*e[13])/w,
				(other.x()*e[2]+other.y()*e[6]+other.z()*e[10]+1*e[14])/w};
}

Mat4 Mat4::translation(const Vec3& trans) {
	return translation(trans.x(), trans.y(), trans.z());
}

Mat4 Mat4::scaling(const Vec3& scale) {
	return scaling(scale.x(), scale.y(), scale.z());
}

Mat4 Mat4::translation(float x, float y, float z) {
	return {1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			x, y, z, 1};
}

Mat4 Mat4::scaling(float x, float y, float z) {
	return {x, 0, 0, 0,
			0, y, 0, 0,
			0, 0, z, 0,
			0, 0, 0, 1};
}

Mat4 Mat4::rotationX(float degree) {
	const float angle{deg2Rad(degree)};
	const float cosAngle = cosf(angle);
	const float sinAngle = sinf(angle);

	return {1, 0, 0, 0,
			0, cosAngle, sinAngle, 0,
			0, -sinAngle, cosAngle, 0,
			0, 0, 0, 1};
}


Mat4 Mat4::rotationY(float degree) {
	const float angle{deg2Rad(degree)};
	const float cosAngle{cosf(angle)};
	const float sinAngle{sinf(angle)};

	return {cosAngle, 0, -sinAngle, 0,
			0, 1, 0, 0,
			sinAngle, 0, cosAngle, 0,
			0, 0, 0, 1};
}


Mat4 Mat4::rotationZ(float degree) {
	const float angle{deg2Rad(degree)};
	const float cosAngle{cosf(angle)};
	const float sinAngle{sinf(angle)};

	return {cosAngle, sinAngle, 0, 0,
			-sinAngle, cosAngle, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1};	
}

Mat4 Mat4::rotationAxis(const Vec3& axis, float degree) {
	const float angle{deg2Rad(degree)};
	const float cosAngle{cosf(angle)};
	const float sinAngle{sinf(angle)};
	const float oneMinusCosAngle{1.0f-cosAngle};

	const Vec3 sqrAxis{axis * axis};

	return {cosAngle+oneMinusCosAngle*sqrAxis.x(), oneMinusCosAngle*axis.x()*axis.y()-sinAngle*axis.z(),oneMinusCosAngle*axis.x()*axis.z()+sinAngle*axis.y(),0,
			oneMinusCosAngle*axis.x()*axis.y()+sinAngle*axis.z(), cosAngle+oneMinusCosAngle*sqrAxis.y(),oneMinusCosAngle*axis.y()*axis.z()-sinAngle*axis.x(),0,
			oneMinusCosAngle*axis.x()*axis.z()-sinAngle*axis.y(), oneMinusCosAngle*axis.y()*axis.z()+sinAngle*axis.x(), cosAngle+oneMinusCosAngle*sqrAxis.z(),0,
			0, 0, 0, 1};
}

Mat4 Mat4::transpose(const Mat4& m) {
	return {m.e[0],m.e[4],m.e[8],m.e[12],
			m.e[1],m.e[5],m.e[9],m.e[13],
			m.e[2],m.e[6],m.e[10],m.e[14],
			m.e[3],m.e[7],m.e[11],m.e[15]};
}

Mat4 Mat4::inverse(const Mat4& m) {

	Mat4 result;

	float Q =   m.e[4] *(m.e[11]*( m.e[1] * m.e[14] - m.e[2]  * m.e[13])+
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

	result.e[0] =  ( m.e[7]  * m.e[9]  * m.e[14]
			+ m.e[15] * m.e[5]  * m.e[10]
			- m.e[15] * m.e[6]  * m.e[9]
			- m.e[11] * m.e[5]  * m.e[14]
			- m.e[7]  * m.e[13] * m.e[10]
			+ m.e[11] * m.e[6]  * m.e[13])/Q;
		result.e[4] = -( m.e[4]  * m.e[15] * m.e[10]
			- m.e[4]  * m.e[11] * m.e[14]
			- m.e[15] * m.e[6]  * m.e[8]
			+ m.e[11] * m.e[6]  * m.e[12]
			+ m.e[7]  * m.e[8]  * m.e[14]
			- m.e[7]  * m.e[12] * m.e[10])/Q;
	result.e[8] = (- m.e[4]  * m.e[11] * m.e[13]
			+ m.e[4]  * m.e[15] * m.e[9]
			- m.e[15] * m.e[8]  * m.e[5]
			- m.e[7]  * m.e[12] * m.e[9]
			+ m.e[11] * m.e[12] * m.e[5]
			+ m.e[7]  * m.e[8]  * m.e[13])/Q;
		result.e[12] =  -(m.e[4]  * m.e[9]  * m.e[14]
			- m.e[4]  * m.e[13] * m.e[10]
			+ m.e[12] * m.e[5]  * m.e[10]
			- m.e[9]  * m.e[6]  * m.e[12]
			- m.e[8]  * m.e[5]  * m.e[14]
			+ m.e[13] * m.e[6]  * m.e[8])/Q;
	/// 2
	result.e[1] = (- m.e[1]  * m.e[15] * m.e[10]
			+ m.e[1]  * m.e[11] * m.e[14]
			- m.e[11] * m.e[2]  * m.e[13]
			- m.e[3]  * m.e[9]  * m.e[14]
			+ m.e[15] * m.e[2]  * m.e[9]
			+ m.e[3]  * m.e[13] * m.e[10])/Q;

	result.e[5] = (- m.e[15] * m.e[2]  * m.e[8]
			+ m.e[15] * m.e[0]  * m.e[10]
			- m.e[11] * m.e[0]  * m.e[14]
			- m.e[3]  * m.e[12] * m.e[10]
			+ m.e[11] * m.e[2]  * m.e[12]
			+ m.e[3]  * m.e[8]  * m.e[14])/Q;

	result.e[9] = -(-m.e[1]  * m.e[15] * m.e[8]
			+ m.e[1]  * m.e[11] * m.e[12]
			+ m.e[15] * m.e[0]  * m.e[9]
			- m.e[3]  * m.e[9]  * m.e[12]
			+ m.e[3]  * m.e[13] * m.e[8]
			- m.e[11] * m.e[0]  * m.e[13])/Q;

	result.e[13] = (- m.e[1]  * m.e[8]  * m.e[14]
			+ m.e[1]  * m.e[12] * m.e[10]
			+ m.e[0]  * m.e[9]  * m.e[14]
			- m.e[0]  * m.e[13] * m.e[10]
			- m.e[12] * m.e[2]  * m.e[9]
			+ m.e[8]  * m.e[2]  * m.e[13])/Q;
	/// 3
	result.e[2] = -( m.e[15] * m.e[2]  * m.e[5]
			- m.e[7]  * m.e[2]  * m.e[13]
			- m.e[3]  * m.e[5]  * m.e[14]
			+ m.e[1]  * m.e[7]  * m.e[14]
			- m.e[1]  * m.e[15] * m.e[6]
			+ m.e[3]  * m.e[13] * m.e[6])/Q;

	result.e[6] = (- m.e[4]  * m.e[3]  * m.e[14]
			+ m.e[4]  * m.e[15] * m.e[2]
			+ m.e[7]  * m.e[0]  * m.e[14]
			- m.e[15] * m.e[6]  * m.e[0]
			- m.e[7]  * m.e[12] * m.e[2]
			+ m.e[3]  * m.e[6]  * m.e[12])/Q;

	result.e[10] = -(-m.e[15] * m.e[0]  * m.e[5]
			+ m.e[15] * m.e[1]  * m.e[4]
			+ m.e[3]  * m.e[12] * m.e[5]
			+ m.e[7]  * m.e[0]  * m.e[13]
			- m.e[7]  * m.e[1]  * m.e[12]
			- m.e[3]  * m.e[4]  * m.e[13])/Q;

	result.e[14] = -( m.e[14] * m.e[0]  * m.e[5]
			- m.e[14] * m.e[1]  * m.e[4]
			- m.e[2]  * m.e[12] * m.e[5]
			- m.e[6]  * m.e[0]  * m.e[13]
			+ m.e[6]  * m.e[1]  * m.e[12]
			+ m.e[2]  * m.e[4]  * m.e[13])/Q;
	/// 4
	result.e[3] = (- m.e[1]  * m.e[11] * m.e[6]
			+ m.e[1]  * m.e[7]  * m.e[10]
			- m.e[7]  * m.e[2]  * m.e[9]
			- m.e[3]  * m.e[5]  * m.e[10]
			+ m.e[11] * m.e[2]  * m.e[5]
			+ m.e[3]  * m.e[9]  * m.e[6])/Q;

	result.e[7] = -(-m.e[4]  * m.e[3]  * m.e[10]
			+ m.e[4]  * m.e[11] * m.e[2]
			+ m.e[7]  * m.e[0]  * m.e[10]
			- m.e[11] * m.e[6]  * m.e[0]
			+ m.e[3]  * m.e[6]  * m.e[8]
			- m.e[7]  * m.e[8]  * m.e[2])/Q;

	result.e[11] = (- m.e[11] * m.e[0]  * m.e[5]
			+ m.e[11] * m.e[1]  * m.e[4]
			+ m.e[3]  * m.e[8]  * m.e[5]
			+ m.e[7]  * m.e[0]  * m.e[9]
			- m.e[7]  * m.e[1]  * m.e[8]
			- m.e[3]  * m.e[4]  * m.e[9])/Q;

	result.e[15] =  ( m.e[10] * m.e[0]  * m.e[5]
			- m.e[10] * m.e[1]  * m.e[4]
			- m.e[2]  * m.e[8]  * m.e[5]
			- m.e[6]  * m.e[0]  * m.e[9]
			+ m.e[6]  * m.e[1]  * m.e[8]
			+ m.e[2]  * m.e[4]  * m.e[9])/Q;
	return result;
}


float Mat4::deg2Rad(const float d) {
	return M_PI*d/180.0f;
}

Mat4 Mat4::perspective(float fovy, float aspect, float znear, float zfar) {
	float cotan {1.0f/tanf(Mat4::deg2Rad(fovy)/2.0f)};

	return {cotan/aspect, 0.0f, 0.0f, 0.0f,
			0.0f, cotan, 0.0f, 0.0f,
			0.0f, 0.0f, -(zfar+znear)/(zfar-znear), -1.0f,
			0.0f, 0.0f, -2.0f*(zfar*znear)/(zfar-znear), 0.0f};
}


Mat4 Mat4::ortho(float left, float right, float bottom, float top, float znear, float zfar ) {
	return {2.0f/(right-left), 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f/(top-bottom), 0.0f, 0.0f,
			0.0f, 0.0f, -2.0f/(zfar-znear), 0.0f,
			-(right+left)/(right-left), -(top+bottom)/(top-bottom), -(zfar+znear)/(zfar-znear), 1.0f};
}
