#include "ReverseParticleSystem.h"
#include "Rand.h"


ReverseParticleSystem::ReverseParticleSystem(const std::shared_ptr<Bitmap> targetBitmap,
                                             const Vec3& initialSpeedMin,
                                             const Vec3& initialSpeedMax,
                                             const Vec3& acceleration,
                                             float maxAge, float pointSize,
                                             const Vec3& color, bool autorestart) :
    AbstractParticleSystem(pointSize, autorestart),
    targetBitmap(targetBitmap),
    initialSpeedMin(initialSpeedMin),
    initialSpeedMax(initialSpeedMax),
    acceleration(acceleration),
    maxAge(maxAge),
    color(color)
{
    
}
    

void ReverseParticleSystem::update(float t) {
    
}

void ReverseParticleSystem::setBitmap(const Bitmap& targetBitmap) {
    
}

void ReverseParticleSystem::setColor(const Vec3& color) {
    
}

void ReverseParticleSystem::restart(size_t count) {
    
}

void ReverseParticleSystem::setAcceleration(const Vec3& acceleration) {
    
}

std::vector<float> ReverseParticleSystem::getData() const {
    std::vector<float> dummy;
    
    for (size_t i = 0;i<10;++i) {
        dummy.push_back(Rand::rand01());
        dummy.push_back(Rand::rand01());
        dummy.push_back(Rand::rand01());
        dummy.push_back(1);
        dummy.push_back(1);
        dummy.push_back(1);
        dummy.push_back(1);
    }
    
    return dummy;
}

size_t ReverseParticleSystem::getParticleCount() const {
    return 10000;
}
