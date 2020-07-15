#include "ParticleSystem.h"
#include "Rand.h"
#include "Vec2.h"

ParticleSystem::ParticleSystem(	uint32_t particleCount, std::shared_ptr<StartVolume> starter,
								const Vec3& initialSpeedMin, const Vec3& initialSpeedMax, 
								const Vec3& acceleration, const Vec3& minPos, const Vec3& maxPos, 
								float maxAge, float pointSize, const Vec3& color, bool autorestart,
                                EruptionType eruptionType, std::shared_ptr<Grid2D> grid) :
	AbstractParticleSystem(pointSize),
	starter(starter),
	initialSpeedMin(initialSpeedMin),
	initialSpeedMax(initialSpeedMax),
	particles{},
	acceleration(acceleration),
	color(color),
	maxAge(maxAge),
	lastT{0},
    autorestart{autorestart},
    grid{grid},
    eruptionType{eruptionType}
{	
	for (uint32_t i = 0;i<particleCount;++i) {
		Particle p{computeStart(), computeDirection(), acceleration, computeColor(color), 1.0f, autorestart ? maxAge*Rand::rand01() : 0, minPos, maxPos, true, grid};
		particles.push_back(p);		
	}	
}

std::vector<float> ParticleSystem::getData() const {
	std::vector<float> data;
	for (const Particle& p : particles) {
		std::vector<float> pData{p.getData()};			
		data.insert(data.end(), pData.begin(), pData.end());
	}	
	return data;	
}


void ParticleSystem::setBounce(bool bounce) {
	for (Particle& p : particles) {
		p.setBounce(bounce);
	}	
}

void ParticleSystem::restart(size_t count) {	
	for (Particle& p : particles) {
		if (p.isDead()) {
			p.restart(computeStart(), computeDirection(), acceleration, computeColor(color), 1.0f, maxAge*Rand::rand01());
			count--;
			if (count == 0) break;
		}
	}	
}

void ParticleSystem::update(float t) {
	const float deltaT = t-lastT;
	lastT = t;
#pragma omp parallel for
	for (int i = 0; i < particles.size();++i) {		
		particles[i].update(deltaT);
		if (particles[i].isDead() && getAutoRestart()) {
			particles[i].restart(computeStart(), computeDirection(), acceleration, computeColor(color), 1.0f, maxAge*Rand::rand01());
		}
	}	
}
	
void ParticleSystem::setStarter(const std::shared_ptr<StartVolume> starter) {
	this->starter = starter;
}

void ParticleSystem::setAcceleration(const Vec3& acceleration) {
	this->acceleration = acceleration;	
	for (Particle& p : particles) {
		p.setAcceleration(acceleration);
	}
}

Vec3 ParticleSystem::computeStart() const {
	return starter->getPosition();
}

Vec3 ParticleSystem::computeDirection() const {
	float radius = 1.0f;
	float t = glfwGetTime();

	switch (eruptionType) {
		case DefaultEruption:
			return initialSpeedMin + (initialSpeedMax - initialSpeedMin) * Vec3{ Rand::rand01(),Rand::rand01(),Rand::rand01() };
		case SmoothEruption:
			radius = (initialSpeedMax - initialSpeedMin).length() * (0.6f + 0.4f * Rand::rand01()) * 0.15f;
			return Vec3::randomPointInSphere() * radius + Vec3{ 0.0f, 0.15f, 0.0f };
		case MagicChaoticVulcano:
			radius = (initialSpeedMax - initialSpeedMin).length() * (0.6f + 0.4f * Rand::rand01()) * 0.1f;
			return Vec3::randomPointInSphere() * radius + Vec3{ 0.04f * cos(floorf(t * 3.5f) * 10.0f),
																0.05f + 0.2f * (1.2f + cos(floorf(t * 13.0f))) *
                                                                0.2f * Rand::rand01(),
																0.04f * sin(floorf(t * 4.0f) * 20.0f) }
                                                                *1.0f;
	}


	
}

void ParticleSystem::setColor(const Vec3& color) {
	if (this->color != color) {
		this->color = color;
		for (Particle& p : particles) {
			p.setColor(computeColor(color));
		}
	}	
}

Particle::Particle( const Vec3& position, const Vec3& direction, const Vec3& acceleration, 
					const Vec3& color, float opacity, float maxAge, const Vec3& minPos, const Vec3& maxPos,
					bool bounce, std::shared_ptr<Grid2D> grid) :
	position(position),
	direction(direction),
	acceleration(acceleration),
	color(color),
	opacity(opacity),
	bounce(bounce),
	maxAge(maxAge),
	age(0.0f),
	minPos(minPos),
	maxPos(maxPos),
    grid(grid)
{
}

void Particle::setBounce(bool bounce) {
	this->bounce = bounce;
}

void Particle::update(float deltaT) {	
	age+=deltaT;
	if(isDead()) {
		opacity = 0.0f;
		return;
	}

	Vec3 nextPosition{position + direction*deltaT};
	
    if (grid) {
        
        const Vec2 posOverGrid{(nextPosition.x()-minPos.x())/(maxPos.x()-minPos.x()),
                               (nextPosition.z()-minPos.z())/(maxPos.z()-minPos.z())};
        
        const float gridHeight = grid->sample(posOverGrid);
        
        if (bounce) {
            if (nextPosition.x() < minPos.x() || nextPosition.x() > maxPos.x())
                direction = direction * Vec3(-0.5f,0.0f,0.0f);
            if (nextPosition.y() < gridHeight || nextPosition.y() > maxPos.y())
                direction = direction * Vec3(0.0f,-0.5f,0.0f);
            if (nextPosition.z() < minPos.z() || nextPosition.z() > maxPos.z())
                direction = direction * Vec3(0.0f,0.0f,-0.5f);
            nextPosition = position + direction*deltaT;
        } else {
			const float reduceAir = 0.999f;
			const float reduceHit = 0.4f;
			if (nextPosition.x() < minPos.x() || nextPosition.x() > maxPos.x() ||
                nextPosition.y() < gridHeight || nextPosition.y() > maxPos.y() ||
                nextPosition.z() < minPos.z() || nextPosition.z() > maxPos.z()) {

				Vec3 n = grid->normal(posOverGrid);
				direction = (Vec3::reflect(direction, n))*reduceHit;
				nextPosition = position + direction * deltaT;
				
            }
			direction = direction * reduceAir;
        }
    } else {
        if (bounce) {
            if (nextPosition.x() < minPos.x() || nextPosition.x() > maxPos.x())
                direction = direction * Vec3(-0.5f,0.0f,0.0f);
            if (nextPosition.y() < minPos.y() || nextPosition.y() > maxPos.y())
                direction = direction * Vec3(0.0f,-0.5f,0.0f);
            if (nextPosition.z() < minPos.z() || nextPosition.z() > maxPos.z())
                direction = direction * Vec3(0.0f,0.0f,-0.5f);
            nextPosition = position + direction*deltaT;
        } else {
            if (nextPosition.x() < minPos.x() || nextPosition.x() > maxPos.x() ||
                nextPosition.y() < minPos.y() || nextPosition.y() > maxPos.y() ||
                nextPosition.z() < minPos.z() || nextPosition.z() > maxPos.z()) {
                direction = Vec3(0,0,0);
                acceleration = Vec3(0,0,0);
                nextPosition = position;
            }
        }
    }
	position = nextPosition;
	direction = direction + acceleration*deltaT;	
}
	
std::vector<float> Particle::getData() const {
	Vec3 c = color == RAINBOW_COLOR ? Vec3::hsvToRgb({age*100,1.0,1.0}) : color;
	//return {position.x(), position.y(), position.z(), c.x(), c.y(), c.z(), opacity*((maxAge-age)/maxAge)};
	return { position.x(), position.y(), position.z(), c.x(), c.y(), c.z(), opacity  };
}

void Particle::restart(const Vec3& position, const Vec3& direction, const Vec3& acceleration, const Vec3& color, float opacity, float maxAge) {	
	this->position = position;
	this->direction = direction;
	this->acceleration = acceleration;
	this->color = color;
	this->opacity = opacity;
	this->maxAge = maxAge;
	age = 0.0f;
}
