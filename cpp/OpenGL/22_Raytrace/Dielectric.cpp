#include <cmath>

#include "Material.h"
#include "Hitable.h"
#include "Rand.h"

Dielectric::Dielectric(float refIdx) :
	refIdx{refIdx}
{
}

std::optional<std::pair<Vec3,Ray>> Dielectric::scatter(const Ray& rIn, const HitRecord& hitRec) const {
	const Vec3 color{1.0f,1.0f,1.0f};
	const float etaiOverEtat = hitRec.frontFace ? (1.0f / refIdx) : refIdx;
	
	const Vec3 unitDirection{Vec3::normalize(rIn.direction())};
	const float cosTheta{fminf(Vec3::dot(unitDirection*-1, hitRec.n), 1.0f)};
	const float sinTheta{sqrtf(1.0f - cosTheta*cosTheta)};

	// TIR
	if (etaiOverEtat * sinTheta > 1.0) {
		const Vec3 reflected{Vec3::reflect(unitDirection, hitRec.n)};
		const Ray scattered{hitRec.p, reflected};
		return std::make_pair(color, scattered);
	}

	// reflection
	const float reflectProb {schlick(cosTheta, etaiOverEtat)};
	if (staticRand.rand01() < reflectProb) {
		const Vec3 reflected{Vec3::reflect(unitDirection, hitRec.n)};
		const Ray scattered{hitRec.p, reflected};
		return std::make_pair(color, scattered);
	}
		
	// refraction	
	const Vec3 refracted{Vec3::refract(unitDirection, hitRec.n, etaiOverEtat)};
	const Ray scattered{hitRec.p, refracted};
	return std::make_pair(color, scattered);
}

float Dielectric::schlick(float cosine, float refIdx) {
	float r0{(1.0f-refIdx) / (1.0f+refIdx)};
	r0 = r0*r0;
	return r0 + (1-r0)*powf(1 - cosine, 5);
}
