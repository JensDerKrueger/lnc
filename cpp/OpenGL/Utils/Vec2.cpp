#include <sstream>
#include <cmath>

#include "Vec2.h"
#include "Rand.h"

Vec2::Vec2() :
	e{0,0}
{}

Vec2::Vec2(float x, float y) :  
	e{x,y}
{}
	
Vec2::Vec2(const Vec2& other) :
	e{other.e}
{}

Vec2::Vec2(const Vec2i& other) :
	e{float(other.x()), float(other.y())}
{}
	
const std::string Vec2::toString() const {
	std::stringstream s;
	s << "[" << e[0] << ", " << e[1] << "]";
	return s.str();
}

float Vec2::x() const {
	return e[0];
}

float Vec2::y() const {
	return e[1];
}

Vec2 Vec2::operator+(const Vec2& val) const{
	return Vec2{e[0]+val.e[0],
				e[1]+val.e[1]};
}

Vec2 Vec2::operator-(const Vec2& val) const {
	return Vec2{e[0]-val.e[0],
				e[1]-val.e[1]};
}

Vec2 Vec2::operator*(const Vec2& val) const {
	return Vec2{e[0]*val.e[0],
				e[1]*val.e[1]};	
}

Vec2 Vec2::operator/(const Vec2& val) const {
	return Vec2{e[0]/val.e[0],
				e[1]/val.e[1]};
}

Vec2 Vec2::operator*(const float& val) const {
	return Vec2{e[0]*val,
				e[1]*val};
}

Vec2 Vec2::operator/(const float& val) const {
	return Vec2{e[0]/val,
				e[1]/val};
}

bool Vec2::operator == ( const Vec2& other ) const {
	return e == other.e;
}

bool Vec2::operator != ( const Vec2& other ) const {
	return e != other.e;
}
