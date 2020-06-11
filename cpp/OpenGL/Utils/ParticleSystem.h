#pragma once

#include <vector>
#include <memory>

#include "Rand.h"
#include "Vec3.h"
#include "Mat4.h"
#include "GLProgram.h"
#include "GLBuffer.h"
#include "GLArray.h"
#include "GLTexture2D.h"

class StartVolume {
public:
	virtual Vec3 getPosition() const = 0;
};

class SphereStart : public StartVolume {
public:
	SphereStart(const Vec3& center, float radius) : center(center), radius(radius) {}
	virtual ~SphereStart() {}
	void setStart(const Vec3& center, float radius) {
		this->center = center;
		this->radius = radius;
	}
	virtual Vec3 getPosition() const {
		return center+Vec3::randomPointInSphere()*radius;
	}
private:
	Vec3 center;
	float radius;
};

class BrickStart : public StartVolume {
public:
	BrickStart(const Vec3& center, const Vec3& extend) : center(center), extend(extend) {}
	virtual ~BrickStart() {}
	void setStart(const Vec3& center, const Vec3& extend) {
		this->center = center;
		this->extend = extend;
	}
	virtual Vec3 getPosition() const {
		return center-extend/2.0+Vec3(Rand::rand01(),Rand::rand01(),Rand::rand01())*extend;
	}
private:
	Vec3 center;
	Vec3 extend;
};


class Particle {
public:
	Particle(const Vec3& position, const Vec3& direction, const Vec3& acceleration, 
			 const Vec3& color, float opacity, float maxAge, 
			 const Vec3& minPos, const Vec3& maxPos, bool bounce);
	
	void update(float deltaT);
	void restart(const Vec3& position, const Vec3& direction, const Vec3& acceleration, 
				 const Vec3& color, float opacity, float maxAge);
	bool isDead() const {return age >= maxAge;};
	void setBounce(bool bounce);
	void setColor(const Vec3& color) {this->color = color;}
	void setAcceleration(const Vec3& acceleration) {this->acceleration = acceleration;}
	
	std::vector<float> getData() const;
	
private:
	Vec3 position;
	Vec3 direction;
	Vec3 acceleration;
	Vec3 color;
	float opacity;
	bool bounce;
	
	float maxAge;
	float age;
	
	Vec3 minPos;
	Vec3 maxPos;
};

const Vec3 RANDOM_COLOR{-1.0f,-1.0f,-1.0f};
const Vec3 RAINBOW_COLOR{-2.0f,-2.0f,-2.0f};

class ParticleSystem {
public:
	ParticleSystem(	uint32_t particleCount, std::shared_ptr<StartVolume> starter,
					const Vec3& initialSpeedMin, const Vec3& initialSpeedMax, 
					const Vec3& acceleration, const Vec3& minPos, const Vec3& maxPos,
					float maxAge, float pointSize, const Vec3& color=RANDOM_COLOR, bool autorestart=true);

	void render(const Mat4& v, const Mat4& p);
	void update(float t);
	
	void setStarter(const std::shared_ptr<StartVolume> starter);
	void setSize(float pointSize) {this->pointSize = pointSize;}
	
	void setColor(const Vec3& color);
	
	void setAutRestart(bool autorestart) {this->autorestart = autorestart;}
	
	void restart(size_t count);
	
	void setBounce(bool bounce);
	void setAcceleration(const Vec3& acceleration);
	void setMaxAge(float maxAge) {this->maxAge = maxAge;}
	void setInitialSpeed(const Vec3& initialSpeedMin, const Vec3& initialSpeedMax) {
		this->initialSpeedMin = initialSpeedMin;
		this->initialSpeedMax = initialSpeedMax;
	}

private:
	ParticleSystem(const ParticleSystem&);
	
	std::shared_ptr<StartVolume> starter;
	Vec3 initialSpeedMin;
	Vec3 initialSpeedMax;
	std::vector<Particle> particles;
	
	GLProgram prog;
	GLint mvpLocation;
	GLint posLocation;
	GLint colLocation;
	GLint texLocation;
	
	GLTexture2D sprite;
	
	Vec3 acceleration;
	
	float pointSize;
	Vec3 color;
	float maxAge;
	GLArray particleArray;
	GLBuffer vbPosColor;
	float lastT;
	bool autorestart;
	
	Vec3 computeStart() const;
	Vec3 computeDirection() const;
	Vec3 computeColor() const;
	
};