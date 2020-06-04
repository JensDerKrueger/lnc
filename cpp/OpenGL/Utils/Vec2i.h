#pragma once 

#include <ostream>
#include <string>
#include <array>

class Vec2i {
	public:
		Vec2i();
		Vec2i(uint32_t x, uint32_t y);
		Vec2i(const Vec2i& other);
		
		friend std::ostream& operator<<(std::ostream &os, const Vec2i& v) {os << v.toString() ; return os;}
		const std::string toString() const;
		
		uint32_t x() const;
		uint32_t y() const;
				
		Vec2i operator+(const Vec2i& val) const;
		Vec2i operator-(const Vec2i& val) const;
		Vec2i operator*(const Vec2i& val) const;
		Vec2i operator/(const Vec2i& val) const;

		Vec2i operator*(const uint32_t& val) const;
		Vec2i operator/(const uint32_t& val) const;		
		
		bool operator == ( const Vec2i& other ) const;
		bool operator != ( const Vec2i& other ) const;

		operator uint32_t*(void) {return e.data();}
		operator const uint32_t*(void) const  {return e.data();}
							
		static Vec2i random();		
			
	private:
		std::array<uint32_t, 2> e;
		
};
