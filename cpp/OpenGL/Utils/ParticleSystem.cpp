#include "ParticleSystem.h"
#include "Rand.h"

ParticleSystem::ParticleSystem(	uint32_t particleCount, std::shared_ptr<StartVolume> starter,
								const Vec3& initialSpeedMin, const Vec3& initialSpeedMax, 
								const Vec3& acceleration, const Vec3& minPos, const Vec3& maxPos, 
								float maxAge, float pointSize, const Vec3& color, bool autorestart) :
	AbstractParticleSystem(pointSize),
	starter(starter),
	initialSpeedMin(initialSpeedMin),
	initialSpeedMax(initialSpeedMax),
	particles{},
	acceleration(acceleration),
	color(color),
	maxAge(maxAge),
	lastT{0},
    autorestart{autorestart}
{	
	for (uint32_t i = 0;i<particleCount;++i) {
		Particle p{computeStart(), computeDirection(), acceleration, computeColor(color), 1.0f, autorestart ? maxAge*Rand::rand01() : 0, minPos, maxPos, true};
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
	
	for (Particle& p : particles) {
		p.update(deltaT);
		if (p.isDead() && getAutoRestart()) {
			p.restart(computeStart(), computeDirection(), acceleration, computeColor(color), 1.0f, maxAge*Rand::rand01());
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
	return initialSpeedMin + (initialSpeedMax - initialSpeedMin) * Vec3{Rand::rand01(),Rand::rand01(),Rand::rand01()};
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
					bool bounce) :
	position(position),
	direction(direction),
	acceleration(acceleration),
	color(color),
	opacity(opacity),
	bounce(bounce),
	maxAge(maxAge),
	age(0.0f),
	minPos(minPos),
	maxPos(maxPos)
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
	
	if (bounce) {
		if (nextPosition.x() < minPos.x() || nextPosition.x() > maxPos.x())	direction = direction * Vec3(-0.5f,0.0f,0.0f);
		if (nextPosition.y() < minPos.y() || nextPosition.y() > maxPos.y())	direction = direction * Vec3(0.0f,-0.5f,0.0f);
		if (nextPosition.z() < minPos.z() || nextPosition.z() > maxPos.z())	direction = direction * Vec3(0.0f,0.0f,-0.5f);
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
	position = nextPosition;
	direction = direction + acceleration*deltaT;	
}
	
std::vector<float> Particle::getData() const {
	Vec3 c = color == RAINBOW_COLOR ? Vec3::hsvToRgb({age*100,1.0,1.0}) : color;
	return {position.x(), position.y(), position.z(), c.x(), c.y(), c.z(), opacity*((maxAge-age)/maxAge)};
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
