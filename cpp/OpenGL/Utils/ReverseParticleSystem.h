#pragma once

#include "AbstractParticleSystem.h"

#include <vector>
#include <memory>

#include "Rand.h"

class Bitmap {
public:
    Bitmap(uint32_t width, uint32_t height) :
        width(width),
        height(height),
        data(width*height) {}
    void setBit(uint32_t x, uint32_t y, bool bit) {data[serializeIndex(x,y)] = bit;}
    bool getBit(uint32_t x, uint32_t y) const {return data[serializeIndex(x,y)];}
    
private:
    uint32_t width;
    uint32_t height;
    std::vector<bool> data;
    
    size_t serializeIndex(uint32_t x, uint32_t y) const {return size_t(width*y+x);}
};


class ReverseParticleSystem : public AbstractParticleSystem {
public:
	ReverseParticleSystem(const std::shared_ptr<Bitmap> targetBitmap,
                          const Vec3& initialSpeedMin, const Vec3& initialSpeedMax,
						  const Vec3& acceleration,
						  float maxAge, float pointSize, const Vec3& color=RANDOM_COLOR,
                          bool autorestart=true);

    void update(float t);
    void setBitmap(const Bitmap& targetBitmap);
    void setColor(const Vec3& color);
    void restart(size_t count);
    
    void setAcceleration(const Vec3& acceleration);
    void setMaxAge(float maxAge) {this->maxAge = maxAge;}
    void setInitialSpeed(const Vec3& initialSpeedMin, const Vec3& initialSpeedMax) {
        this->initialSpeedMin = initialSpeedMin;
        this->initialSpeedMax = initialSpeedMax;
    }
    virtual std::vector<float> getData() const;
    virtual size_t getParticleCount() const;
    

private:
	std::shared_ptr<Bitmap> targetBitmap;
	Vec3 initialSpeedMin;
	Vec3 initialSpeedMax;
		
	Vec3 acceleration;
    float maxAge;
	Vec3 color;
	float lastT;

};
