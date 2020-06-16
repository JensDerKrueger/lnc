#include "ReverseParticleSystem.h"
#include "Rand.h"


ReverseParticleSystem::ReverseParticleSystem(uint32_t particleCount,
                                             const std::shared_ptr<Bitmap> targetBitmap,
                                             const Vec3& initialSpeedMin,
                                             const Vec3& initialSpeedMax,
                                             const Vec3& acceleration,
                                             float maxAge, float pointSize,
                                             const Vec3& color, bool autorestart) :
    AbstractParticleSystem(pointSize, autorestart),
    particleCount(particleCount),
    targetBitmap(targetBitmap),
    initialSpeedMin(initialSpeedMin),
    initialSpeedMax(initialSpeedMax),
    acceleration(acceleration),
    maxAge(maxAge),
    color(color)
{
    recomputeTrajectories();
}
    
void ReverseParticleSystem::update(float t) {
    
}

void ReverseParticleSystem::setBitmap(const std::shared_ptr<Bitmap> targetBitmap) {
    this->targetBitmap = targetBitmap;
    recomputeTrajectories();
}

Vec3 ReverseParticleSystem::computeColor() const {
    if (color == RANDOM_COLOR)
        return Vec3::random();
    else
        return color;
}

void ReverseParticleSystem::restart(size_t count) {
    
}

void ReverseParticleSystem::recomputeTrajectories() {
    // computeColor
    
    size_t targetPosCount = 0;
    for (bool p : *targetBitmap) {
        if (p) targetPosCount++;
    }
    
    float particlePerBitmap = float(particleCount)/float(targetPosCount);
    float accParticle = 0;
    
    particleStart.clear();
    
    for (uint32_t y = 0; y < targetBitmap->getHeight(); ++y) {
        for (uint32_t x = 0; x < targetBitmap->getWidth(); ++x) {
            if (targetBitmap->getBit(x,y)) {
                accParticle += particlePerBitmap;
                
                uint32_t spanParticle = uint32_t(accParticle);
                accParticle -= spanParticle;
                for (uint32_t i = 0; i<spanParticle;++i) {
                    particleStart.push_back(Vec3{float(x),float(y),0.0f});
                }
            }
            if (particleStart.size() == particleCount) break;
        }
        if (particleStart.size() == particleCount) break;
    }
    
    if (!particleStart.empty()) {
        while (particleCount>particleStart.size()) {
            particleStart.push_back(particleStart.back());
        }
    }
    
    
    
    
}

void ReverseParticleSystem::setColor(const Vec3& color) {
    this->color = color;
}

void ReverseParticleSystem::setAcceleration(const Vec3& acceleration) {
    this->acceleration = acceleration;
    recomputeTrajectories();
}

std::vector<float> ReverseParticleSystem::getData() const {
    std::vector<float> dummy;
    
    for (size_t i = 0;i<particleCount;++i) {
        dummy.push_back(particleStart[i].x());
        dummy.push_back(particleStart[i].y());
        dummy.push_back(particleStart[i].z());
        
        
        dummy.push_back(1);
        dummy.push_back(1);
        dummy.push_back(1);
        dummy.push_back(1);
    }
    
    return dummy;
}

size_t ReverseParticleSystem::getParticleCount() const {
    return particleCount;
}
