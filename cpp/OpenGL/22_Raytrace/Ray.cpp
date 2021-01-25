#include "Ray.h"


Ray::Ray(const Ray& other) :
	o{other.o},
	d{other.d} 
{}

Ray::Ray(const Vec3& o, const Vec3& d) :
	o{o},
	d{d} 
{}

const Vec3 Ray::origin() const {
	return o;
}

const Vec3 Ray::direction() const {
	return d;
}

const Vec3 Ray::pointAtParameter(float t) const {
	return o + d*t;
}
