#include <cmath>

#include "Sphere.h"

Sphere::Sphere(const Vec3& center, const float radius, const std::shared_ptr<Material> materal) :
	center{center},
	radius{radius},
	material{materal}
{	
}

const std::optional<HitRecord> Sphere::hit(const Ray &r, const float tMin, const float tMax) const {
	
	const float a = r.direction().sqlength();
	const Vec3 oc{r.origin() - center};
	const float halfB = Vec3::dot(oc, r.direction());
	const float c = oc.sqlength() - radius*radius;
	const float discriminant = halfB*halfB - a*c;

	if (discriminant > 0) {
		const float root = sqrt(discriminant);

		const float t1 = (-halfB - root) / a;
		if (t1 < tMax && t1 > tMin) {
			const Vec3 p{r.pointAtParameter(t1)};
			const Vec3 outwardNormal{(p - center) / radius};
			return HitRecord{t1,p,outwardNormal,r,material};
		}
		
		const float t2 = (-halfB + root) / a;
		if (t2 < tMax && t2 > tMin) {
			const Vec3 p{r.pointAtParameter(t2)};
			const Vec3 outwardNormal{(p - center) / radius};
			return HitRecord{t2,p,outwardNormal,r,material};
		}
	}
	
	return {};
}