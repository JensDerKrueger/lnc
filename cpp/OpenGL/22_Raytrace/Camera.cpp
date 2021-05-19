#include <cmath>

#include "Camera.h"

Camera::Camera(const Vec3& lookFrom, const Vec3& lookAt, const Vec3& vUp, float fovyDeg, 
			   float aspectRatio, float aperture, float distToFocus) :
	lookFrom{lookFrom},
	lookAt{lookAt},
	vUp{vUp},
	fovy{3.14159265f * fovyDeg / 180.0f},
	lenseRadius{aperture/2},
	halfHeight{tanf(fovy/2) * distToFocus},
	halfWidth{halfHeight * aspectRatio},
	w{Vec3::normalize(lookAt-lookFrom)},
	u{Vec3::normalize(Vec3::cross(w,vUp))},
	v{Vec3::cross(u,w)},
	lowerLeftCorner{lookFrom + w*distToFocus - v*halfHeight - u*halfWidth}
{
}

Ray Camera::getRay(const float s, const float t) const {
	const Vec3 temp{Vec3::randomPointInDisc() * lenseRadius};
	const Vec3 apertureOffset{u*temp.x + v*temp.y};
	return Ray{lookFrom + apertureOffset, lowerLeftCorner + u*s*2*halfWidth + v*t*2*halfHeight  - (lookFrom + apertureOffset)};
}
