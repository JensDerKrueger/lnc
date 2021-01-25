#include "HitableList.h"

#include <iostream>


HitableList::HitableList() 
{	
}

void HitableList::add(std::shared_ptr<const Hitable> hitable) {
	objects.push_back(hitable);
}

const std::optional<HitRecord>HitableList::hit(const Ray& r, const float tMin, const float tMax) const {
	float tClosest = tMax;
	std::optional<HitRecord> hit{};
	for (auto object : objects) {
		std::optional<HitRecord> tmpHit{object->hit(r, tMin, tClosest)};
		if (tmpHit) {
			tClosest = tmpHit->t;
			hit = tmpHit.value();
		}
	}
	return hit;
}

