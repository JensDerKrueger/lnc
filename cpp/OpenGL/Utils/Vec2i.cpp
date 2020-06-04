#include <sstream>
#include <cmath>

#include "Vec2i.h"
#include "Rand.h"

Vec2i::Vec2i() :
	e{0,0}
{}

Vec2i::Vec2i(int32_t x, int32_t y) :  
	e{x,y}
{}
	
Vec2i::Vec2i(const Vec2i& other) :
	e{other.e}
{}
	
const std::string Vec2i::toString() const {
	std::stringstream s;
	s << "[" << e[0] << ", " << e[1] << "]";
	return s.str();
}

int32_t Vec2i::x() const {
	return e[0];
}

int32_t Vec2i::y() const {
	return e[1];
}

Vec2i Vec2i::operator+(const Vec2i& val) const{
	return Vec2i{e[0]+val.e[0],
				e[1]+val.e[1]};
}

Vec2i Vec2i::operator-(const Vec2i& val) const {
	return Vec2i{e[0]-val.e[0],
				e[1]-val.e[1]};
}

Vec2i Vec2i::operator*(const Vec2i& val) const {
	return Vec2i{e[0]*val.e[0],
				e[1]*val.e[1]};	
}

Vec2i Vec2i::operator/(const Vec2i& val) const {
	return Vec2i{e[0]/val.e[0],
				e[1]/val.e[1]};
}

Vec2i Vec2i::operator*(const int32_t& val) const {
	return Vec2i{e[0]*val,
				e[1]*val};
}

Vec2i Vec2i::operator/(const int32_t& val) const {
	return Vec2i{e[0]/val,
				e[1]/val};
}

bool Vec2i::operator == ( const Vec2i& other ) const {
	return e == other.e;
}

bool Vec2i::operator != ( const Vec2i& other ) const {
	return e != other.e;
}