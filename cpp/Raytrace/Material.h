#pragma once

#include <utility>
#include <optional>

#include "Vec3.h"
#include "Ray.h"

class HitRecord;

class Material {
public:
	virtual std::optional<std::pair<Vec3,Ray>> scatter(const Ray& rIn, const HitRecord& hitRec) const = 0;
};
	
class Dielectric : public Material {
public:	
	Dielectric(float refIdx);
	virtual ~Dielectric() {}
	virtual std::optional<std::pair<Vec3,Ray>> scatter(const Ray& rIn, const HitRecord& hitRec) const;
private:
	float refIdx;	
	static float schlick(float cosine, float refIdx);
	
};

class Lambertian : public Material {
public:	
	Lambertian(const Vec3& color);
	virtual ~Lambertian() {}
	virtual std::optional<std::pair<Vec3,Ray>> scatter(const Ray& rIn, const HitRecord& hitRec) const;
private:
	Vec3 color;
};

class Metal : public Material {
public:	
	Metal(const Vec3& color, float fuzz);
	virtual ~Metal() {}
	virtual std::optional<std::pair<Vec3,Ray>> scatter(const Ray& rIn, const HitRecord& hitRec) const;	
private:
	Vec3 color;
	float fuzz;
};