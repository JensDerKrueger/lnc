#include "Hitable.h"
#include "Material.h"

HitRecord::HitRecord(const HitRecord& other) :
	t{other.t},
	p{other.p},
	r{other.r},
	material{other.material},
	frontFace{other.frontFace},
	n{other.n}
{}

HitRecord::HitRecord(const float t, const Vec3& p, const Vec3& n, const Ray& r,
					 const std::shared_ptr<Material> material) :				
	t{t},
	p{p},
	r{r},
	material{material},
	frontFace{Vec3::dot(r.direction(), n) < 0},
	n{frontFace ? n : n*-1}
{}