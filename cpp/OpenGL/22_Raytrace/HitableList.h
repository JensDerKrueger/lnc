#pragma once

#include <memory>
#include <vector>

#include "Ray.h"
#include "Hitable.h"

class HitableList {
	public:
		HitableList();
		void add(std::shared_ptr<const Hitable> hitable);
		const std::optional<HitRecord> hit(const Ray& r, const float tMin, const float tMax) const;
		
	private:
		std::vector<std::shared_ptr<const Hitable>> objects;
};
