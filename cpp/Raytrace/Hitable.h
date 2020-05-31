#pragma once 

#include <memory>

#include "Vec3.h"
#include "Ray.h"
#include "Material.h"

class HitRecord {
	public:
		HitRecord(const HitRecord& other);
		HitRecord(const float t, const Vec3& p, const Vec3& n, const Ray& r,
				  const std::shared_ptr<Material> material);				

		float t;
		Vec3 p;
		Ray r;
		std::shared_ptr<Material> material;
		bool frontFace;
		Vec3 n;
};


class Hitable {
	public:
		virtual const std::optional<HitRecord> hit(const Ray& r, const float tMin, const float tMax) const = 0;
};
