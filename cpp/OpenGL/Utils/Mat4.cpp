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

Mat4::Mat4(const Vec3& e1, float e14, const Vec3& e2, float e24, const Vec3& e3, float e34, const Vec3& e4, float e44) :
	Mat4(e1.x(),e1.y(),e1.z(),e14,
		   e2.x(),e2.y(),e2.z(),e24,
		   e3.x(),e3.y(),e3.z(),e34,
		   e4.x(),e4.y(),e4.z(),e44)
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

Mat4::operator float*(void) {
  return e.data();
}

Mat4::operator const float*(void) const  {
  return e.data();
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
			result.e[x+y] = other.e[0+x] * e[0+y]+
                      other.e[1+x] * e[4+y]+
                      other.e[2+x] * e[8+y]+
                      other.e[3+x] * e[12+y];
	return result;
}

Vec3 Mat4::operator * ( const Vec3& other ) const {
	float w = other.x()*e[12]+other.y()*e[13]+other.z()*e[14]+1*e[15];
	return Vec3{(other.x()*e[0]+other.y()*e[1]+other.z()*e[2]+1*e[3])/w,
              (other.x()*e[4]+other.y()*e[5]+other.z()*e[6]+1*e[7])/w,
              (other.x()*e[8]+other.y()*e[9]+other.z()*e[10]+1*e[11])/w};
}

Vec4 Mat4::operator * ( const Vec4& other ) const {
  return Vec4{(other.x()*e[0]+other.y()*e[1]+other.z()*e[2]+other.w()*e[3]),
              (other.x()*e[4]+other.y()*e[5]+other.z()*e[6]+other.w()*e[7]),
              (other.x()*e[8]+other.y()*e[9]+other.z()*e[10]+other.w()*e[11]),
              (other.x()*e[12]+other.y()*e[13]+other.z()*e[14]+other.w()*e[15])};
}

Mat4 Mat4::translation(const Vec3& trans) {
	return translation(trans.x(), trans.y(), trans.z());
}

Mat4 Mat4::scaling(const Vec3& scale) {
	return scaling(scale.x(), scale.y(), scale.z());
}

Mat4 Mat4::translation(float x, float y, float z) {
	return {1, 0, 0, x,
          0, 1, 0, y,
          0, 0, 1, z,
          0, 0, 0, 1};
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
	const float a{deg2Rad(degree)};
	const float cosA{cosf(a)};
	const float sinA{sinf(a)};
	const float omCosA{1.0f-cosA};

	const Vec3 sqrAxis{axis * axis};

  return {cosA+omCosA*sqrAxis.x(),                 omCosA*axis.x()*axis.y()+sinA*axis.z(), omCosA*axis.x()*axis.z()-sinA*axis.y(), 0,
          omCosA*axis.x()*axis.y()-sinA*axis.z(),  cosA+omCosA*sqrAxis.y(),                omCosA*axis.y()*axis.z()+sinA*axis.x(), 0,
          omCosA*axis.x()*axis.z()+sinA*axis.y(),  omCosA*axis.y()*axis.z()-sinA*axis.x(), cosA+omCosA*sqrAxis.z(),                0,
          0,                                       0,                                      0,                                      1
  };
}

Mat4 Mat4::transpose(const Mat4& m) {
	return {m.e[0],m.e[4],m.e[8],m.e[12],
          m.e[1],m.e[5],m.e[9],m.e[13],
          m.e[2],m.e[6],m.e[10],m.e[14],
          m.e[3],m.e[7],m.e[11],m.e[15]};
}

Mat4 Mat4::inverse(const Mat4& m) {
  return Mat4::inverse(m, Mat4::det(m));
}

float Mat4::det(const Mat4& m) {
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

Mat4 Mat4::inverse(const Mat4& m, float det) {
   float Q = 1.0f/det;

   Mat4 result;
  
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
 
float Mat4::deg2Rad(const float d) {
	return M_PI*d/180.0f;
}


Mat4 Mat4::perspective(float fovy, float aspect, float znear, float zfar) {
	float cotan {1.0f/tanf(Mat4::deg2Rad(fovy)/2.0f)};

  return {cotan/aspect, 0.0f, 0.0f, 0.0f,
          0.0f, cotan, 0.0f, 0.0f,
          0.0f, 0.0f, -(zfar+znear)/(zfar-znear), -2.0f*(zfar*znear)/(zfar-znear),
          0.0f, 0.0f, -1.0f, 0.0f};
}

Mat4 Mat4::perspective(float left, float right, float bottom, float top, float znear, float zfar) {
  return {2.0f*znear/(right-left),   0.0f,                      (right+left)/(right-left),       0.0f,
          0.0f,                      2.0f*znear/(top-bottom),   (top+bottom)/(top-bottom),       0.0f,
          0.0f,                      0.0f,                      -(zfar+znear)/(zfar-znear),     -2.0f*(zfar*znear)/(zfar-znear),
          0.0f,                      0.0f,                      -1.0f,                           0.0f};

}

Mat4 Mat4::ortho(float left, float right, float bottom, float top, float znear, float zfar ) {
	return {2.0f/(right-left), 0.0f, 0.0f, -(right+left)/(right-left),
          0.0f, 2.0f/(top-bottom), 0.0f, -(top+bottom)/(top-bottom),
          0.0f, 0.0f, -2.0f/(zfar-znear), -(zfar+znear)/(zfar-znear),
          0.0f, 0.0f, 0.0f, 1.0f};
}


Mat4 Mat4::lookAt(const Vec3& vEye, const Vec3& vAt, const Vec3& vUp) {
	Vec3 f{vAt-vEye};
	Vec3 u{vUp};
	Vec3 s{Vec3::cross(f,u)};
	u = Vec3::cross(s,f);

	f = Vec3::normalize(f);
	u = Vec3::normalize(u);
	s = Vec3::normalize(s);

	return {s.x(), s.y(), s.z(), -Vec3::dot(s,vEye),
          u.x(), u.y(), u.z(), -Vec3::dot(u,vEye),
          -f.x(), -f.y(), -f.z(), Vec3::dot(f,vEye),
          0.0f,0.0f,0.0f,1.0};
}

Mat4 Mat4::mirror(const Vec3& p, const Vec3& n) {	
	float k = Vec3::dot(p,n);

  
  return {1-2*n.x()*n.x(),-2*n.x()*n.y(),-2*n.x()*n.z(),2*k*n.x(),
    -2*n.y()*n.x(),1-2*n.y()*n.y(),-2*n.y()*n.z(),2*k*n.y(),
    -2*n.z()*n.x(),-2*n.z()*n.y(),1-2*n.z()*n.z(),2*k*n.z(),
    0,0,0,1
  };
}

StereoMatrices Mat4::stereoLookAtAndProjection(const Vec3& eye, const Vec3& at, const Vec3& up,
                                               float fovy, float aspect, float znear, float zfar,
                                               float focalLength, float eyeDist) {
  
  StereoMatrices result;
    
  float wd2     = znear * tanf(Mat4::deg2Rad(fovy)/2.0f);
  float nfdl    = znear / focalLength;
  float shift   =   eyeDist * nfdl;
  float top     =   wd2;
  float bottom  = - wd2;

  // projection matrices
  float left    = - aspect * wd2 - shift;
  float right   =   aspect * wd2 - shift;
  result.leftProj = Mat4::perspective(left, right, bottom, top, znear, zfar);
  left    = - aspect * wd2 + shift;
  right   =   aspect * wd2 + shift;
  result.rightProj = Mat4::perspective(left, right, bottom, top, znear, zfar);
  
  // view matrices
  result.leftView  = Mat4::lookAt(eye, at, up) * Mat4::translation(-eyeDist/2.0f, 0.0f, 0.0f);
  result.rightView = Mat4::lookAt(eye, at, up) * Mat4::translation(eyeDist/2.0f, 0.0f, 0.0f);
  
  return result;
}
