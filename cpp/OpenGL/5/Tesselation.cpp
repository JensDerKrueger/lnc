#include <cmath>

constexpr float PI = 3.14159265358979323846f;

#include "Tesselation.h"

Tesselation Tesselation::genSphere(const Vec3& center, const float radius, const uint32_t sectorCount, const uint32_t stackCount) {
	Tesselation tess{};
	
	const float lengthInv{1.0f / radius};
	const float sectorStep{2.0f * PI / sectorCount};
	const float stackStep{PI / stackCount};

	for(uint32_t i = 0; i <= stackCount; ++i) {
		const float stackAngle{PI / 2.0f - i * stackStep};       // starting from pi/2 to -pi/2
		const float xy{radius * cosf(stackAngle)};             // r * cos(u)
		const float z{radius * sinf(stackAngle)};              // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for(uint32_t j = 0; j <= sectorCount; ++j){
			const float sectorAngle{j * sectorStep};           // starting from 0 to 2pi

			// vertex position (x, y, z)
			float x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			float y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
			tess.vertices.push_back(center.x() + x);
			tess.vertices.push_back(center.y() + y);
			tess.vertices.push_back(center.z() + z);

			// normalized vertex normal (nx, ny, nz)
			tess.normals.push_back(x * lengthInv);
			tess.normals.push_back(y * lengthInv);
			tess.normals.push_back(z * lengthInv);

			// vertex tex coord (s, t) range between [0, 1]
			tess.texCoords.push_back((float)j / sectorCount);
			tess.texCoords.push_back((float)i / stackCount);
		}
	}
	
	
	// generate CCW index list of sphere triangles
	for(uint32_t i = 0; i < stackCount; ++i) {
		uint32_t k1 = i * (sectorCount + 1);     // beginning of current stack
		uint32_t k2 = k1 + sectorCount + 1;      // beginning of next stack

		for(uint32_t j = 0; j < sectorCount; ++j, ++k1, ++k2) {
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if(i != 0) {
				tess.indices.push_back(k1);
				tess.indices.push_back(k2);
				tess.indices.push_back(k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if(i != (stackCount-1)){
				tess.indices.push_back(k1 + 1);
				tess.indices.push_back(k2);
				tess.indices.push_back(k2 + 1);
			}
		}
	}
	
	return tess;
}