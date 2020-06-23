#pragma once

#include "Vec3.h"
#include "Mat4.h"

#include "GLProgram.h"
#include "GLBuffer.h"
#include "GLArray.h"
#include "GLTexture2D.h"

const Vec3 RANDOM_COLOR{-1.0f,-1.0f,-1.0f};
const Vec3 RAINBOW_COLOR{-2.0f,-2.0f,-2.0f};

class AbstractParticleSystem {
public:
	AbstractParticleSystem(float pointSize);
	virtual void update(float t) = 0;
		
	void setPointSize(float pointSize) {this->pointSize = pointSize;}	
	float getPointSize() const {return pointSize;}	
	virtual void setColor(const Vec3& color) = 0;
	
	void render(const Mat4& v, const Mat4& p);
	
	virtual std::vector<float> getData() const = 0;
	virtual size_t getParticleCount() const = 0;

    static Vec3 computeColor(const Vec3& c);
    
private:
	float pointSize;
	
	GLProgram prog;
	GLint mvpLocation;
	GLint texLocation;	
	GLTexture2D sprite;	
	
	GLArray particleArray;
	GLBuffer vbPosColor;

};
