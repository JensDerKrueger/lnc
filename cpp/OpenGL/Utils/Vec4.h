#pragma once 

#include <ostream>
#include <string>
#include <array>

#include "Vec3.h"

class Vec4 {
	public:
		Vec4();
		Vec4(float x, float y, float z, float w);
		Vec4(const Vec4& other);
        Vec4(const Vec3& other, float w);
		
		friend std::ostream& operator<<(std::ostream &os, const Vec4& v) {os << v.toString() ; return os;}
		const std::string toString() const;
		
		float x() const;
		float y() const;
		float z() const;
        float w() const;

		float r() const;
		float g() const;
		float b() const;
        float a() const;

		Vec4 operator+(const Vec4& val) const;
		Vec4 operator-(const Vec4& val) const;
		Vec4 operator*(const Vec4& val) const;
		Vec4 operator/(const Vec4& val) const;

		Vec4 operator*(const float& val) const;
		Vec4 operator/(const float& val) const;
		
		bool operator == ( const Vec4& other ) const;
		bool operator != ( const Vec4& other ) const;

		float length() const;
		float sqlength() const;
    
    Vec3 vec3() const;
		
		operator float*(void) {return e.data();}
		operator const float*(void) const  {return e.data();}
			
		static float dot(const Vec4& a, const Vec4& b);
		static Vec4 normalize(const Vec4& a);
				
		static Vec4 random();
					
	private:
		std::array<float, 4> e;
		
		
};
