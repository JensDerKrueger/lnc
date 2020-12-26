#include "MC.h"
#include "MC.inl"

Isosurface::Isosurface(const Volume& volume, uint8_t isovalue) {
  for (size_t l = 0; l < volume.depth-1; ++l) {
    for(size_t v = 0; v < volume.height-1; v++) {
      for(size_t u = 0; u < volume.width-1; u++) {
      
        // fetch data from the volume
        std::array<uint8_t, 8> data;
        std::array<Vec3, 8> normals;
        for (uint8_t i = 0;i<8;++i) {
          const size_t index = (u+vertexPosTable[i][0]) + (v+vertexPosTable[i][1]) * volume.width + (l+vertexPosTable[i][2]) * volume.width*volume.height;
          data[i]    = volume.data[index];
          normals[i] = volume.normals[index];
        }
        
        // classify vertices and compute case index
        uint8_t mcCase{0};
        for (uint8_t i = 0;i<8;++i) mcCase += uint8_t(data[i] < isovalue) * (1<<i);
        
        // bail out on empty cases
        if (mcCase == 0 || mcCase == 255) continue;
        
        // compute offset to current cube
        const Vec3 cubeOffset{
          float(u)/volume.maxSize-0.5f*float(volume.width)/float(volume.maxSize),
          float(v)/volume.maxSize-0.5f*float(volume.height)/float(volume.maxSize),
          float(l)/volume.maxSize-0.5f*float(volume.depth)/float(volume.maxSize)
        };
        
        // interpolate vertices and normals
        std::array<Vertex, 12> trisVertices;
        for (uint8_t i = 0;i<12;++i) {
          if(edgeTable[mcCase] & 1<<i) {
            const std::array<uint8_t,2>& index = edgeToVertexTable[i];
            const float alpha = std::max(0.0f, std::min(1.0f, (float(data[index[0]])-float(isovalue)) / (float(data[index[0]])-float(data[index[1]]))));
            const Vec3 positionInCube{vertexPosTable[index[0]] + ( vertexPosTable[index[1]]-vertexPosTable[index[0]]) * alpha};
            const Vec3 normal{normals[index[0]] + (normals[index[1]]-normals[index[0]]) * alpha};
            trisVertices[i] = Vertex{Vec3{volume.scale * (cubeOffset + positionInCube/float(volume.maxSize))}, Vec3::normalize(normal)};
          }
        }
       
        // add triangles to isosurface
        size_t i = 0;
        while (trisTable[mcCase][i] != N_E) {
          vertices.push_back(trisVertices[trisTable[mcCase][i++]]);
          vertices.push_back(trisVertices[trisTable[mcCase][i++]]);
          vertices.push_back(trisVertices[trisTable[mcCase][i++]]);
        }
      }
    }
  }
}
