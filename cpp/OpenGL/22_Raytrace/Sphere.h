#pragma once

#include <optional>
#include <memory>

#include "Vec3.h"
#include "Ray.h"
#include "Hitable.h"
#include "Material.h"

class Sphere : public Hitable {
	public:
		Sphere(const Vec3& center, const float radius, const std::shared_ptr<Material> material);
		virtual ~Sphere() {}
		virtual const std::optional<HitRecord> hit(const Ray& r, const float tMin, const float tMax) const;
		
	private:
		const Vec3 center;
		const float radius;
		const std::shared_ptr<Material> material;

};