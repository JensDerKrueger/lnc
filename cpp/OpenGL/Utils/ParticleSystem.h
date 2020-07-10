#pragma once

#include "AbstractParticleSystem.h"

#include <vector>
#include <memory>

#include "Rand.h"
#include "Grid2D.h"

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
			 const Vec3& minPos, const Vec3& maxPos, bool bounce, std::shared_ptr<Grid2D> grid);
	
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
    std::shared_ptr<Grid2D> grid;
};

class ParticleSystem : public AbstractParticleSystem {
public:
	ParticleSystem(	uint32_t particleCount, std::shared_ptr<StartVolume> starter,
					const Vec3& initialSpeedMin, const Vec3& initialSpeedMax, 
					const Vec3& acceleration, const Vec3& minPos, const Vec3& maxPos,
					float maxAge, float pointSize, const Vec3& color=RANDOM_COLOR,
                    bool autorestart=true, std::shared_ptr<Grid2D> grid=nullptr);

	void update(float t);
	
	void setStarter(const std::shared_ptr<StartVolume> starter);
	
	void setColor(const Vec3& color);
		
	void restart(size_t count);
	
	void setBounce(bool bounce);
	void setAcceleration(const Vec3& acceleration);
	void setMaxAge(float maxAge) {this->maxAge = maxAge;}
	void setInitialSpeed(const Vec3& initialSpeedMin, const Vec3& initialSpeedMax) {
		this->initialSpeedMin = initialSpeedMin;
		this->initialSpeedMax = initialSpeedMax;
	}
	
	virtual std::vector<float> getData() const;
	virtual size_t getParticleCount() const {return particles.size();}

    void setAutoRestart(bool autorestart) {this->autorestart = autorestart;}
    bool getAutoRestart() const {return autorestart;}

private:
	ParticleSystem(const ParticleSystem&);
	
	std::shared_ptr<StartVolume> starter;
	Vec3 initialSpeedMin;
	Vec3 initialSpeedMax;
	std::vector<Particle> particles;
		
	Vec3 acceleration;
	
	Vec3 color;
	float maxAge;
	float lastT;
    bool autorestart;
    std::shared_ptr<Grid2D> grid;
	
	Vec3 computeStart() const;
	Vec3 computeDirection() const;
};
