#pragma once 

#include "Ray.h"

class Camera {
	public:
		Camera(const Vec3& lookFrom, const Vec3& lookAt, const Vec3& vUp, float fovy, 
			   float aspectRatio, float aperture, float distToFocus);
		Ray getRay(const float s, const float t) const;
		
	private:		
		Vec3 lookFrom;
		Vec3 lookAt;
		Vec3 vUp;
		float fovy;
		float lenseRadius;
		
		float halfHeight;
		float halfWidth;
		
		Vec3 w;
		Vec3 u;
		Vec3 v;

		Vec3 lowerLeftCorner;
};