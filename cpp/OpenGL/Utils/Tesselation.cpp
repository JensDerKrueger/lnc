#include <cmath>
#include <iostream>

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
			const float x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			const float y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
			tess.vertices.push_back(center.x() + x);
			tess.vertices.push_back(center.y() + y);
			tess.vertices.push_back(center.z() + z);

			// normalized vertex normal (nx, ny, nz)
			tess.normals.push_back(x * lengthInv);
			tess.normals.push_back(y * lengthInv);
			tess.normals.push_back(z * lengthInv);

			const float nextSectorAngle{(j+1) * sectorStep};
			const float nx = xy * cosf(nextSectorAngle);
			const float ny = xy * sinf(nextSectorAngle);
			
			// compute the tangent an make sure it is perpendicular to the normal
			Vec3 n{x * lengthInv, y * lengthInv,z * lengthInv};
			Vec3 t{Vec3::normalize(Vec3{nx,ny,z}-Vec3{x,y,z})};
			Vec3 b{Vec3::cross(n,t)};
			Vec3 tCorr{Vec3::cross(b,n)};
					
			// normalized vertex tangent (tx, ty, tz)
			tess.tangents.push_back(tCorr.x());
			tess.tangents.push_back(tCorr.y());
			tess.tangents.push_back(tCorr.z());
			
			// vertex tex coord (s, t) range between [0, 1]
			tess.texCoords.push_back((float)j / sectorCount);
			tess.texCoords.push_back(1.0-(float)i / stackCount);
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


Tesselation Tesselation::genRectangle(const Vec3& center, const float width, const float height) {
	const Vec3 u{width/2.0f,0.0,0.0};
	const Vec3 v{0.0,height/2.0f,0.0,};	
	return genRectangle(center-u-v,center+u-v,center+u+v,center-u+v);
}

//   D ----- C
//   |       |
//   |       |
//   A ----- B
Tesselation Tesselation::genRectangle(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d) {
	Tesselation tess{};
	
	const Vec3 u{b-a};
	const Vec3 v{c-a};
		
	tess.vertices = std::vector<float>{
		a.x(), a.y(), a.z(),
		b.x(), b.y(), b.z(),
		c.x(), c.y(), c.z(),
		d.x(), d.y(), d.z(),
	};	
	
	const Vec3 normal{Vec3::normalize(Vec3::cross(u,v))};

	tess.normals = std::vector<float>{
		normal.x(), normal.y(), normal.z(), 
		normal.x(), normal.y(), normal.z(), 
		normal.x(), normal.y(), normal.z(), 
		normal.x(), normal.y(), normal.z() 
	};

	const Vec3 tangent{Vec3::normalize(u)};
	tess.tangents = std::vector<float>{
		tangent.x(), tangent.y(), tangent.z(), 
		tangent.x(), tangent.y(), tangent.z(), 
		tangent.x(), tangent.y(), tangent.z(), 
		tangent.x(), tangent.y(), tangent.z()
	};

	tess.texCoords = std::vector<float>{
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};
	
	tess.indices = std::vector<uint32_t>{0,1,2,0,2,3};
	
	return tess;
}

//      H ----- G
//     /|      /|
//    / |     / |
//   D -+--- C  F
//   | / E   | /
//   |/      |/
//   A ----- B
Tesselation Tesselation::genBrick(const Vec3& center, const Vec3& size) {
	Tesselation tess{};


	const Vec3 E = center-size/2.0f;
	const Vec3 C = center+size/2.0f;

	const Vec3 A{E.x(), E.y(), C.z()};
	const Vec3 B{C.x(), E.y(), C.z()};
	const Vec3 D{E.x(), C.y(), C.z()};
	const Vec3 F{C.x(), E.y(), E.z()};
	const Vec3 G{C.x(), C.y(), E.z()};
	const Vec3 H{E.x(), C.y(), E.z()};

	tess.vertices = std::vector<float>{
		// front
		A.x(), A.y(), A.z(),
		B.x(), B.y(), B.z(),
		C.x(), C.y(), C.z(),
		D.x(), D.y(), D.z(),

		// back
		F.x(), F.y(), F.z(),
		E.x(), E.y(), E.z(),
		H.x(), G.y(), H.z(),
		G.x(), G.y(), G.z(),

		// left
		E.x(), E.y(), E.z(),
		A.x(), A.y(), A.z(),
		D.x(), D.y(), D.z(),
		H.x(), H.y(), H.z(),

		// right
		B.x(), B.y(), B.z(),
		F.x(), F.y(), F.z(),
		G.x(), G.y(), G.z(),
		C.x(), C.y(), C.z(),

		// top
		D.x(), D.y(), D.z(),
		C.x(), C.y(), C.z(),
		G.x(), G.y(), G.z(),
		H.x(), H.y(), H.z(),

		// bottom
		B.x(), B.y(), B.z(),
		A.x(), A.y(), A.z(),
		E.x(), E.y(), E.z(),
		F.x(), F.y(), F.z()
	};

	tess.normals = std::vector<float>{
		// front
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// back
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,

		// left
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,

		// right
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// top
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// bottom
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f
	};
	
	tess.tangents = std::vector<float>{
		// front
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// back
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,

		// left
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// right
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,

		// top
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// bottom
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f
	};
	
	tess.texCoords = std::vector<float>{
		// front
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,

		// back
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,

		// left
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,

		// right
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,

		// top
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,

		// bottom
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	tess.indices = std::vector<uint32_t>{
		0,1,2,0,2,3,
		4,5,6,4,6,7,
		
		8,9,10,8,10,11,
		12,13,14,12,14,15,
		
		16,17,18,16,18,19,
		20,21,22,20,22,23
	};
	
	return tess;
}