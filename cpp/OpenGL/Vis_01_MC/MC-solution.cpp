#include "MC.h"
#include "MC.inl"

#include <iostream>

void Isosurface::mc(const Volume& volume, uint8_t isovalue) {
  vertices.clear();
  for (size_t l = 0;l<volume.depth-1;++l) {
    mcLayer(l, volume, isovalue);
  }
}

void Isosurface::mcLayer(size_t l, const Volume& volume, uint8_t isovalue) {
  const size_t layerSize = volume.width*volume.height;
  
  for(int v = 0; v < volume.height-1; v++) {
    for(int u = 0; u < volume.width-1; u++) {
      
      // fetch data from the volume
      std::array<uint8_t, 8> data;
      std::array<Vec3, 8> normals;
      for (uint8_t i = 0;i<8;++i) {
        const std::array<uint8_t,3>& index = vertexPosTable[i];
        data[i]    = volume.data[(u+index[0]) + (v+index[1]) * volume.width + (l+index[2]) * layerSize];
        normals[i] = volume.normals[(u+index[0]) + (v+index[1]) * volume.width + (l+index[2]) * layerSize];
      }

      // classify vertices and compute case index
      uint8_t cellIndex{0};
      for (uint8_t i = 0;i<8;++i) cellIndex += uint8_t(data[i] < isovalue) * (1<<i);
      
      if (cellIndex == 0 || cellIndex == 255) continue;
      
      // offset to current cube
      const Vec3 cubeOffset{
        2.0f*float(u)/volume.width-1.0f,
        2.0f*float(v)/volume.height-1.0f,
        2.0f*float(l)/volume.depth-1.0f
      };
      
      // interpolate vertices
      std::array<Vertex, 12> trisVertices;
      for (uint8_t i = 0;i<12;++i) {
        if(edgeTable[cellIndex] & 1<<i) {
          const std::array<uint8_t,2>& index = edgeToVertexTable[i];
          const float alpha = std::max(0.0f, std::min(1.0f, (float(data[index[0]])-float(isovalue)) / (float(data[index[0]])-float(data[index[1]]))));
          const Vec3 fromVertex{ float(vertexPosTable[index[0]][0]), float(vertexPosTable[index[0]][1]), float(vertexPosTable[index[0]][2]) };
          const Vec3 toVertex{ float(vertexPosTable[index[1]][0]), float(vertexPosTable[index[1]][1]), float(vertexPosTable[index[1]][2]) };
          
          const Vec3 positionInCube{fromVertex + (toVertex-fromVertex) * alpha};
          const Vec3 normal{normals[index[0]] + (normals[index[1]]-normals[index[0]]) * alpha};
          
          trisVertices[i] = Vertex{Vec3{volume.scale * (cubeOffset + positionInCube*2.0f/volume.maxSize)}, Vec3::normalize(normal)};
        }
      }
     
      // add triangles to isosurface
      size_t i = 0;
      while (trisTable[cellIndex][i] != N_E) {
        vertices.push_back(trisVertices[trisTable[cellIndex][i++]]);
        vertices.push_back(trisVertices[trisTable[cellIndex][i++]]);
        vertices.push_back(trisVertices[trisTable[cellIndex][i++]]);
      }
    }
  }
}

