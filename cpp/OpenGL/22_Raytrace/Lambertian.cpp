#include "Material.h"
#include "Hitable.h"

Lambertian::Lambertian(const Vec3& color) :
	color{color}
{
}

std::optional<std::pair<Vec3,Ray>> Lambertian::scatter(const Ray& rIn, const HitRecord& hitRec) const {
	const Vec3 scatterDirection{hitRec.n + Vec3::randomUnitVector()};
	const Ray scattered{hitRec.p, scatterDirection};
	return std::make_pair(color, scattered);
}