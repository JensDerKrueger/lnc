#pragma once 

#include <ostream>
#include <string>
#include <array>

class Vec2 {
	public:
		Vec2();
		Vec2(float x, float y);
		Vec2(const Vec2& other);
		
		friend std::ostream& operator<<(std::ostream &os, const Vec2& v) {os << v.toString() ; return os;}
		const std::string toString() const;
		
		float x() const;
		float y() const;
				
		Vec2 operator+(const Vec2& val) const;
		Vec2 operator-(const Vec2& val) const;
		Vec2 operator*(const Vec2& val) const;
		Vec2 operator/(const Vec2& val) const;

		Vec2 operator*(const float& val) const;
		Vec2 operator/(const float& val) const;		
		
		bool operator == ( const Vec2& other ) const;
		bool operator != ( const Vec2& other ) const;

		operator float*(void) {return e.data();}
		operator const float*(void) const  {return e.data();}
							
		static Vec2 random();		
			
	private:
		std::array<float, 2> e;
		
};
