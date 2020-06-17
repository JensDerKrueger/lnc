#include <algorithm>

#include "ReverseParticleSystem.h"
#include "Rand.h"
#include "bmp.h"

Bitmap::Bitmap(const std::string& bmpImage, uint8_t threshold) {
    BMP::Image image{BMP::load(bmpImage)};
    width = image.width;
    height = image.height;
    
    data.resize(width*height);
    for (size_t i = 0;i<width*height;++i) {
        data[i] = image.data[i*image.componentCount] >= threshold;
    }
}

ReverseParticleSystem::ReverseParticleSystem(uint32_t particleCount,
                                             const std::shared_ptr<Bitmap> targetBitmap,
                                             const Vec3& initialSpeedMin,
                                             const Vec3& initialSpeedMax,
                                             const Vec3& acceleration,
                                             float maxAge, float pointSize,
                                             const Vec3& color, bool autorestart,
                                             bool reverse) :
    AbstractParticleSystem(pointSize, autorestart),
    particleCount(particleCount),
    targetBitmap(targetBitmap),
    initialSpeedMin(initialSpeedMin),
    initialSpeedMax(initialSpeedMax),
    acceleration(acceleration),
    maxAge(maxAge),
    color(color),
    reverse(reverse)
{
    setColor(color);
    recomputeTrajectories();
}
    
void ReverseParticleSystem::update(float t) {
    const float deltaT = t-lastT;
    lastT = t;
    // TODO
}

void ReverseParticleSystem::setBitmap(const std::shared_ptr<Bitmap> targetBitmap) {
    this->targetBitmap = targetBitmap;
    recomputeTrajectories();
}

void ReverseParticleSystem::restart(size_t count) {
    
}

void ReverseParticleSystem::recomputeTrajectories() {
    particleCountPerTimestep.clear();
    particleColors.clear();
    particlePositions.clear();
    
    size_t targetPosCount = 0;
    for (bool p : *targetBitmap) {
        if (p) targetPosCount++;
    }
    
    float particlePerBitmap = float(particleCount)/float(targetPosCount);
    float accParticle = 0;
    
    std::vector<Vec3> particleStart;
    
    for (uint32_t y = 0; y < targetBitmap->getHeight(); ++y) {
        for (uint32_t x = 0; x < targetBitmap->getWidth(); ++x) {
            if (targetBitmap->getBit(x,y)) {
                accParticle += particlePerBitmap;
                
                uint32_t spanParticle = uint32_t(accParticle);
                accParticle -= spanParticle;
                for (uint32_t i = 0; i<spanParticle;++i) {
                    particleStart.push_back(Vec3{float(x)/(targetBitmap->getWidth()),
                                                 float(y)/(targetBitmap->getHeight()),0.0f});
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
            
    iMaxAge = 0;
    std::vector<uint32_t> maxAges(particleStart.size());
    for (uint32_t i = 0; i < maxAges.size(); ++i) {
        float z = Rand::rand01();
        maxAges[i] = uint32_t(maxAge*(0.6+0.4*z));
        iMaxAge = std::max(maxAges[i], iMaxAge);
    }

    std::vector<Vec3> directions(particleStart.size());
    particlePositions.resize(particleStart.size());
    for (uint32_t i = 0; i < particleStart.size(); ++i) {
        particlePositions[i].push_back(particleStart[i]);
        directions[i] = initialSpeedMin +
                                (initialSpeedMax - initialSpeedMin) *
                                Vec3{Rand::rand01(),Rand::rand01(),Rand::rand01()};
    }
    
    particleCountPerTimestep.push_back(particleStart.size());
    
    for (uint32_t step = 1;step < iMaxAge;++step) {
        size_t active = 0;
        for (uint32_t i = 0; i < particleStart.size(); ++i) {
            if (step > maxAges[i]) {
                continue;
            }
            active++;
            const Vec3& oldPos = particlePositions[i][step-1];
            Vec3 newPos = oldPos + directions[i]*0.0001f;
            directions[i] = directions[i] + acceleration;
            particlePositions[i].push_back(newPos);
        }
        particleCountPerTimestep.push_back(active);
    }
}

void ReverseParticleSystem::setColor(const Vec3& color) {
    this->color = color;
    particleColors.clear();
    for (uint32_t i = 0; i < particleCount; ++i) {
        particleColors.push_back(computeColor(color));
    }
}

void ReverseParticleSystem::setAcceleration(const Vec3& acceleration) {
    this->acceleration = acceleration;
    recomputeTrajectories();
}

std::vector<float> ReverseParticleSystem::getData() const {
    std::vector<float> dummy;
    
    uint32_t iLastT{uint32_t(lastT*1000)};
    uint32_t activeT = (iLastT >= iMaxAge) ? iMaxAge-1 : iLastT;

    for (size_t i = 0;i<particleCount;++i) {
        size_t index = (reverse) ? (iMaxAge-1)-activeT : activeT;

        if (index < particlePositions[i].size()) {
            
            dummy.push_back(particlePositions[i][index].x());
            dummy.push_back(particlePositions[i][index].y());
            dummy.push_back(particlePositions[i][index].z());
            
            Vec3 c = particleColors[i] == RAINBOW_COLOR ? Vec3::hsvToRgb({float(index),1.0,1.0}) : particleColors[i];
            
            dummy.push_back(c.x());
            dummy.push_back(c.y());
            dummy.push_back(c.z());
            dummy.push_back(1);
        }
    }
    
    return dummy;
}

size_t ReverseParticleSystem::getParticleCount() const {
    uint32_t iLastT{uint32_t(lastT*1000)};
    uint32_t activeT = (iLastT >= iMaxAge) ? iMaxAge-1 : iLastT;
    size_t index = (reverse) ? (iMaxAge-1)-activeT : activeT;
    return particleCountPerTimestep[index];
}
