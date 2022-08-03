#include <cmath>
#include <iostream>

#include "Vec2.h"

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
			tess.vertices.push_back(center.x + x);
			tess.vertices.push_back(center.y + y);
			tess.vertices.push_back(center.z + z);

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
			tess.tangents.push_back(tCorr.x);
			tess.tangents.push_back(tCorr.y);
			tess.tangents.push_back(tCorr.z);
			
			// vertex tex coord (s, t) range between [0, 1]
			tess.texCoords.push_back((float)j / sectorCount);
			tess.texCoords.push_back(1.0f-(float)i / stackCount);
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


Tesselation Tesselation::genGrid(const Vec3& center, const float width,
                                 const float height, const uint32_t subDivU,
                                 const uint32_t subDivV) {
  
  Tesselation grid{};
  

  const size_t vertCount = (subDivU+1) * (subDivV+1);
  const size_t indCount  = subDivU * subDivV * 6;

  const Vec3 u{width/2.0f,0.0,0.0};
  const Vec3 v{0.0,height/2.0f,0.0,};
  const Vec3 normal{Vec3::normalize(Vec3::cross(u,v))};
  const Vec3 tangent{Vec3::normalize(u)};

  grid.vertices = std::vector<float>(vertCount*3);
  grid.normals = std::vector<float>(vertCount*3);
  grid.tangents = std::vector<float>(vertCount*3);
  grid.texCoords = std::vector<float>(vertCount*2);

  for (size_t y = 0;y<subDivV+1;++y){
    for (size_t x = 0;x<subDivU+1;++x){
      const size_t i{x + y * (subDivU+1)};
      float normX{float(x) / subDivU};
      float normY{float(y) / subDivV};

      grid.vertices[i*3+0] = center.x - width/2.0f + normX * width;
      grid.vertices[i*3+1] = center.y - height/2.0f + normY * height;
      grid.vertices[i*3+2] = center.z;

      grid.normals[i*3+0] = normal.x;
      grid.normals[i*3+1] = normal.y;
      grid.normals[i*3+2] = normal.z;

      grid.tangents[i*3+0] = tangent.x;
      grid.tangents[i*3+1] = tangent.y;
      grid.tangents[i*3+2] = tangent.z;

      grid.texCoords[i*2+0] = normX;
      grid.texCoords[i*2+1] = normY;
    }
  }

  grid.indices = std::vector<uint32_t>(indCount);
  
  for (size_t y = 0;y<subDivV;++y){
    for (size_t x = 0;x<subDivU;++x){
      const size_t i{x + y * subDivU};
      const size_t j{x + y * (subDivU+1)};
      
      grid.indices[i*6+0] = j;
      grid.indices[i*6+1] = j+1;
      grid.indices[i*6+2] = j+subDivU+1;
      grid.indices[i*6+3] = j+subDivU+1;
      grid.indices[i*6+4] = j+1;
      grid.indices[i*6+5] = j+subDivU+2;
    }
  }
  
  return grid;
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
		a.x, a.y, a.z,
		b.x, b.y, b.z,
		c.x, c.y, c.z,
		d.x, d.y, d.z,
	};	
	
	const Vec3 normal{Vec3::normalize(Vec3::cross(u,v))};

	tess.normals = std::vector<float>{
		normal.x, normal.y, normal.z,
		normal.x, normal.y, normal.z,
		normal.x, normal.y, normal.z,
		normal.x, normal.y, normal.z
	};

	const Vec3 tangent{Vec3::normalize(u)};
	tess.tangents = std::vector<float>{
		tangent.x, tangent.y, tangent.z,
		tangent.x, tangent.y, tangent.z,
		tangent.x, tangent.y, tangent.z,
		tangent.x, tangent.y, tangent.z
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
Tesselation Tesselation::genBrick(const Vec3& center, const Vec3& size, const Vec3& texScale) {
	Tesselation tess{};

	const Vec3 E = center-size/2.0f;
	const Vec3 C = center+size/2.0f;

	const Vec3 A{E.x, E.y, C.z};
	const Vec3 B{C.x, E.y, C.z};
	const Vec3 D{E.x, C.y, C.z};
	const Vec3 F{C.x, E.y, E.z};
	const Vec3 G{C.x, C.y, E.z};
	const Vec3 H{E.x, C.y, E.z};

	tess.vertices = std::vector<float>{
		// front
		A.x, A.y, A.z,
		B.x, B.y, B.z,
		C.x, C.y, C.z,
		D.x, D.y, D.z,

		// back
		F.x, F.y, F.z,
		E.x, E.y, E.z,
		H.x, H.y, H.z,
		G.x, G.y, G.z,

		// left
		E.x, E.y, E.z,
		A.x, A.y, A.z,
		D.x, D.y, D.z,
		H.x, H.y, H.z,

		// right
		B.x, B.y, B.z,
		F.x, F.y, F.z,
		G.x, G.y, G.z,
		C.x, C.y, C.z,

		// top
		D.x, D.y, D.z,
		C.x, C.y, C.z,
		G.x, G.y, G.z,
		H.x, H.y, H.z,

		// bottom
		B.x, B.y, B.z,
		A.x, A.y, A.z,
		E.x, E.y, E.z,
		F.x, F.y, F.z
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
		texScale.x, 0.0f,
		texScale.x, texScale.y,
		0.0f, texScale.y,

		// back
		0.0f, 0.0f,
		texScale.x, 0.0f,
		texScale.x, texScale.y,
		0.0f, texScale.y,

		// left
		0.0f, 0.0f,
		texScale.z, 0.0f,
		texScale.z, texScale.y,
		0.0f, texScale.y,

		// right
		0.0f, 0.0f,
		texScale.z, 0.0f,
		texScale.z, texScale.y,
		0.0f, texScale.y,

		// top
		0.0f, 0.0f,
		texScale.x, 0.0f,
		texScale.x, texScale.z,
		0.0f, texScale.z,

		// bottom
		0.0f, 0.0f,
		texScale.x, 0.0f,
		texScale.x, texScale.z,
		0.0f, texScale.z
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

// Torus
// can be moved around by center, but the normal to the torus plane is always (0, 0, 1)
Tesselation Tesselation::genTorus(const Vec3 &center, float majorRadius,
                                  float minorRadius, uint32_t majorSteps,
                                  uint32_t minorSteps) {
	Tesselation tess{};

	for (uint32_t x = 0; x <= majorSteps; x++) {
		const float phi = (2.0f * PI * x) / majorSteps;
		for (uint32_t y = 0; y <= minorSteps; y++) {
			const float theta = (2.0f * PI * y) / minorSteps;

			const Vec3 vertice{(majorRadius + minorRadius*std::cos(theta))*std::cos(phi),
					   (majorRadius + minorRadius*std::cos(theta))*std::sin(phi),
					   minorRadius * std::sin(theta)};
			const Vec3 normal{Vec3::normalize(vertice - Vec3{majorRadius*std::cos(phi), majorRadius*std::sin(phi), 0})};
			// select tangent in toroidal direction
			const Vec3 tangent{-majorRadius*std::sin(phi), majorRadius*std::cos(phi), 0};
			const Vec2 texture{static_cast<float>(x)/static_cast<float>(majorSteps),
                         static_cast<float>(y)/static_cast<float>(minorSteps)};


			tess.vertices.push_back(vertice.x + center.x);
			tess.vertices.push_back(vertice.y + center.y);
			tess.vertices.push_back(vertice.z + center.z);

			tess.normals.push_back(normal.x);
			tess.normals.push_back(normal.y);
			tess.normals.push_back(normal.z);

			tess.tangents.push_back(tangent.x);
			tess.tangents.push_back(tangent.y);
			tess.tangents.push_back(tangent.z);

			tess.texCoords.push_back(texture.x);
			tess.texCoords.push_back(texture.y);
		}
	}

	for (uint32_t x = 0; x < majorSteps; x++) {
		for (uint32_t y = 0; y < minorSteps; y++) {
			// push 2 triangles per point
			tess.indices.push_back((x+0)*(minorSteps+1) + (y+0));
			tess.indices.push_back((x+1)*(minorSteps+1) + (y+0));
			tess.indices.push_back((x+1)*(minorSteps+1) + (y+1));

			tess.indices.push_back((x+0)*(minorSteps+1) + (y+0));
			tess.indices.push_back((x+1)*(minorSteps+1) + (y+1));
			tess.indices.push_back((x+0)*(minorSteps+1) + (y+1));
		}
	}

	return tess;
}

Tesselation Tesselation::genDisc(const Vec3 &center, float radius,
                                 const uint32_t steps, bool ccw) {
  Tesselation tess{};
  
  for (uint32_t x = 0; x <= steps; x++) {
    const float phi = (2.0f * PI * x) / steps;
    const Vec3 vertex{radius*std::cos(phi),
                      0,
                      radius*std::sin(phi)};
    
    const Vec3 tangent{-radius*std::sin(phi),
                       0,
                       radius*std::cos(phi)};
    
    tess.vertices.push_back(vertex.x + center.x);
    tess.vertices.push_back(vertex.y + center.y);
    tess.vertices.push_back(vertex.z + center.z);

    tess.normals.push_back(0);
    tess.normals.push_back(ccw ? -1 : 1);
    tess.normals.push_back(0);

    tess.tangents.push_back(0);
    tess.tangents.push_back(0);
    tess.tangents.push_back(1);

    tess.texCoords.push_back(std::cos(phi));
    tess.texCoords.push_back(std::sin(phi));
  }
  
  tess.vertices.push_back(center.x);
  tess.vertices.push_back(center.y);
  tess.vertices.push_back(center.z);

  tess.normals.push_back(0);
  tess.normals.push_back(ccw ? -1 : 1);
  tess.normals.push_back(0);

  tess.tangents.push_back(0);
  tess.tangents.push_back(0);
  tess.tangents.push_back(1);

  tess.texCoords.push_back(0.5f);
  tess.texCoords.push_back(0.5f);
  
  if (ccw) {
    for (uint32_t x = 0; x < steps; x++) {
      tess.indices.push_back(x+0);
      tess.indices.push_back(x+1);
      tess.indices.push_back(tess.vertices.size()/3-1);
    }
  } else {
    for (uint32_t x = 0; x < steps; x++) {
      tess.indices.push_back(tess.vertices.size()/3-1);
      tess.indices.push_back(x+1);
      tess.indices.push_back(x+0);
    }
  }
  
  return tess;
}


// can be moved around by center, but the normal
// to the cylinder plane is always (0, 0, 1)
Tesselation Tesselation::genCylinder(const Vec3 &center, const float radius,
                                     const float height, const bool genBottom,
                                     const bool genTop, const uint32_t steps)
{
  Tesselation tess{};
  
  const float texScale = 1.0f / (1+genBottom+genTop);
  float texOffset = 0.0f;
  
  for (uint32_t x = 0; x <= steps; x++) {
    const float phi = (2.0f * PI * x) / steps;

    const Vec3 vertexBottom{
      radius*std::cos(phi),
      -height/2.0f,
      radius*std::sin(phi)
    };
    
    const Vec3 vertexTop{
      radius*std::cos(phi),
      height/2.0f,
      radius*std::sin(phi)
    };
    
    const Vec3 normal = {
      std::cos(phi),
      0,
      std::sin(phi)
    };

    const Vec3 tangent{
      -radius*std::sin(phi),
      0,
      radius*std::cos(phi)
    };
    
    const Vec2 textureBottom{
      static_cast<float>(x)/static_cast<float>(steps),
      0
    };
    const Vec2 textureTop{
      static_cast<float>(x)/static_cast<float>(steps),
      1
    };
    
    tess.vertices.push_back(vertexBottom.x + center.x);
    tess.vertices.push_back(vertexBottom.y + center.y);
    tess.vertices.push_back(vertexBottom.z + center.z);

    tess.vertices.push_back(vertexTop.x + center.x);
    tess.vertices.push_back(vertexTop.y + center.y);
    tess.vertices.push_back(vertexTop.z + center.z);

    tess.normals.push_back(normal.x);
    tess.normals.push_back(normal.y);
    tess.normals.push_back(normal.z);

    tess.normals.push_back(normal.x);
    tess.normals.push_back(normal.y);
    tess.normals.push_back(normal.z);

    tess.tangents.push_back(tangent.x);
    tess.tangents.push_back(tangent.y);
    tess.tangents.push_back(tangent.z);

    tess.tangents.push_back(tangent.x);
    tess.tangents.push_back(tangent.y);
    tess.tangents.push_back(tangent.z);

    tess.texCoords.push_back(textureBottom.x*texScale);
    tess.texCoords.push_back(textureBottom.y);

    tess.texCoords.push_back(textureTop.x*texScale);
    tess.texCoords.push_back(textureTop.y);
  }

  for (uint32_t x = 0; x < steps; x++) {
    tess.indices.push_back(x*2+0);
    tess.indices.push_back(x*2+1);
    tess.indices.push_back(x*2+2);

    tess.indices.push_back(x*2+2);
    tess.indices.push_back(x*2+1);
    tess.indices.push_back(x*2+3);
  }

  if (genBottom) {
    Tesselation disc = Tesselation::genDisc(center-Vec3(0,height/2.0f,0),
                                            radius, steps, true);
    texOffset += texScale;
    tess.append(disc,{texOffset,0.0f},{texScale,1.0f});
  }
  
  if (genTop) {
    Tesselation disc = Tesselation::genDisc(center+Vec3(0,height/2.0f,0),
                                            radius, steps, false);
    texOffset += texScale;
    tess.append(disc,{texOffset,0.0f},{texScale,1.0f});
  }
  
  return tess;
}


void Tesselation::append(const Tesselation& other,
                         const Vec2& texOffset,
                         const Vec2& texScale) {
  const size_t offset = vertices.size()/3;

  vertices.insert(vertices.end(), other.vertices.begin(), other.vertices.end());
  normals.insert(normals.end(), other.normals.begin(), other.normals.end());
  tangents.insert(tangents.end(), other.tangents.begin(), other.tangents.end());

  
  for (size_t i = 0;i< other.texCoords.size();++i) {
    texCoords.push_back(other.texCoords[i]*texScale[i%2]+texOffset[i%2]);
  }

  for (const uint32_t index : other.indices) {
    indices.push_back(offset+index);
  }
}
