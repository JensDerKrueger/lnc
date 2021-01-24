#include "Material.h"

#include "Hitable.h"


Metal::Metal(const Vec3& color, float fuzz) :
	color{color},
	fuzz{fuzz}
{
}

std::optional<std::pair<Vec3,Ray>> Metal::scatter(const Ray& rIn, const HitRecord& hitRec) const {
	const Vec3 reflected{Vec3::reflect(Vec3::normalize(rIn.direction()), hitRec.n)};
	const Ray scattered{hitRec.p, reflected + Vec3::randomUnitVector()*fuzz};
	if (Vec3::dot(scattered.direction(), hitRec.n) > 0)
		return std::make_pair(color, scattered);
	else
		return {};
}