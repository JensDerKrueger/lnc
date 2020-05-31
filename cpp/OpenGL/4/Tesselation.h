#pragma once

#include "Vec3.h"
#include <vector>

class Tesselation {
	public:
		static Tesselation genSphere(const Vec3& center, const float radius, const uint32_t sectorCount, const uint32_t stackCount);

		std::vector<float> vertices;
		std::vector<float> normals;
		std::vector<float> texCoords;
		std::vector<uint32_t> indices;
		
	private:
		Tesselation() {}

};