#pragma once 

#include "Vec3.h"

class Ray {
	public:
		Ray(const Ray& other);
		Ray(const Vec3& o, const Vec3& d);
		
		const Vec3 origin() const;
		const Vec3 direction() const;
		const Vec3 pointAtParameter(float t) const;
	
	private:
		Vec3 o;
		Vec3 d;
};