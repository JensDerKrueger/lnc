#pragma once 

#include <ostream>
#include <string>
#include <array>

class Vec3 {
	public:
		Vec3();
		Vec3(float x, float y, float z);
		Vec3(const Vec3& other);
		
		friend std::ostream& operator<<(std::ostream &os, const Vec3& v) {os << v.toString() ; return os;}
		const std::string toString() const;
		
		float x() const;
		float y() const;
		float z() const;
		
		float r() const;
		float g() const;
		float b() const;
		
		Vec3 operator+(const Vec3& val) const;
		Vec3 operator-(const Vec3& val) const;
		Vec3 operator*(const Vec3& val) const;
		Vec3 operator/(const Vec3& val) const;

		Vec3 operator*(const float& val) const;
		Vec3 operator/(const float& val) const;		
		
		bool operator == ( const Vec3& other ) const;
		bool operator != ( const Vec3& other ) const;

		float length() const;
		float sqlength() const;
		
		operator float*(void) {return e.data();}
		operator const float*(void) const  {return e.data();}
			
		static float dot(const Vec3& a, const Vec3& b);		
		static Vec3 cross(const Vec3& a, const Vec3& b);
		static Vec3 normalize(const Vec3& a);
		
		static Vec3 reflect(const Vec3& a, const Vec3& n);
		static Vec3 refract(const Vec3& a, const Vec3& n, 
							const float index);
        
    static Vec3 minV(const Vec3& a, const Vec3& b);
    static Vec3 maxV(const Vec3& a, const Vec3& b);
    
		static Vec3 random();		
		static Vec3 randomPointInSphere();
		static Vec3 randomPointInHemisphere();
		static Vec3 randomPointInDisc();
		static Vec3 randomUnitVector();
		
		static Vec3 hsvToRgb(const Vec3& other);
			
	private:
		std::array<float, 3> e;
		
		
};
