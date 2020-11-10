#include <iostream>
#include <sstream>
#include <cmath>

#include "Vec4.h"
#include "Rand.h"

Vec4::Vec4() :
	e{0,0,0,0}
{}

Vec4::Vec4(float x, float y, float z, float w) :
	e{x,y,z,w}
{}
	
Vec4::Vec4(const Vec4& other) :
	e{other.e}
{}
	
Vec4::Vec4(const Vec3& other, float w) :
    e{other.x(),other.y(),other.z(),w}
{}

Vec4::Vec4(const Vec2& other, float z, float w) :
  e{other.x(),other.y(),z,w}
{}

const std::string Vec4::toString() const {
	std::stringstream s;
	s << "[" << e[0] << ", " << e[1] << ", " << e[2] << ", " << e[3] << "]";
	return s.str();
}

float Vec4::x() const {
	return e[0];
}

float Vec4::y() const {
	return e[1];
}

float Vec4::z() const {
	return e[2];
}

float Vec4::w() const {
    return e[3];
}

float Vec4::r() const {
	return e[0];
}

float Vec4::g() const {
	return e[1];
}

float Vec4::b() const {
	return e[2];
}

float Vec4::a() const {
    return e[3];
}

Vec4 Vec4::operator+(const Vec4& val) const{
	return Vec4{e[0]+val.e[0],
				e[1]+val.e[1],
				e[2]+val.e[2],
                e[3]+val.e[3]
    };
}

Vec4 Vec4::operator-(const Vec4& val) const {
	return Vec4{e[0]-val.e[0],
				e[1]-val.e[1],
                e[2]-val.e[2],
				e[3]-val.e[3]};
}

Vec4 Vec4::operator*(const Vec4& val) const {
	return Vec4{e[0]*val.e[0],
				e[1]*val.e[1],
				e[2]*val.e[2],
                e[3]*val.e[3]};
}

Vec4 Vec4::operator/(const Vec4& val) const {
	return Vec4{e[0]/val.e[0],
				e[1]/val.e[1],
                e[2]/val.e[2],
				e[3]/val.e[3]};
}

Vec4 Vec4::operator*(const float& val) const {
	return Vec4{e[0]*val,
				e[1]*val,
                e[2]*val,
				e[3]*val};
}

Vec4 Vec4::operator/(const float& val) const {
	return Vec4{e[0]/val,
				e[1]/val,
                e[2]/val,
				e[3]/val};
}


bool Vec4::operator == ( const Vec4& other ) const {
	return e == other.e;
}

bool Vec4::operator != ( const Vec4& other ) const {
	return e != other.e;
}

float Vec4::length() const {
	return sqrt(sqlength());
}

float Vec4::sqlength() const {
	return e[0]*e[0]+e[1]*e[1]+e[2]*e[2]+e[3]*e[3];
}

Vec3 Vec4::vec3() const {
    return Vec3(e[0], e[1], e[2]);
}

float Vec4::dot(const Vec4& a, const Vec4& b) {
	return a.e[0]*b.e[0]+a.e[1]*b.e[1]+a.e[2]*b.e[2]+a.e[3]*b.e[3];
}

Vec4 Vec4::normalize(const Vec4& a) {
	return a/a.length();
}

Vec4 Vec4::random() {
	return Vec4{Rand::rand01(),Rand::rand01(),Rand::rand01(),Rand::rand01()};
}

Vec3 Vec4::xyz() const {
  return Vec3(e[0],e[1],e[2]);
}

Vec2 Vec4::xy() const {
  return Vec2(e[0],e[1]);
}
