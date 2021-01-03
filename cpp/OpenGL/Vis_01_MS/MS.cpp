#include "MS.h"
#include "MS.inl"

Isoline::Isoline(const Image& image, uint8_t isovalue, bool useAsymptoticDecider) {
  for(size_t v = 0; v < image.height-1; v++) {
    for(size_t u = 0; u < image.width-1; u++) {
      // fetch data from the image
      std::array<uint8_t, 4> data{};
      for (uint8_t i = 0;i<4;++i) {
        const size_t index = (u+size_t(vertexPosTable[i][0])) +
                             (v+size_t(vertexPosTable[i][1])) * image.width;
        data[i]    = image.data[index*image.componentCount];
      }
      
      // classify vertices and compute case index
      uint8_t msCase{0};
      for (uint8_t i = 0;i<4;++i) msCase += uint8_t(data[i] > isovalue) * (1<<i);
      
      // bail out on empty cases
      if (msCase == 0 || msCase == 15) continue;
      
      // compute offset to current cube
      const Vec2 squareOffset{
        (float(u)+0.5f)/(image.width),
        (float(v)+0.5f)/(image.height)
      };
      
      // interpolate vertices
      for (uint8_t i = 0;i<4;++i) {
        if(edgeTable[msCase] & 1<<i) {
          const std::array<uint8_t,2>& index = edgeToVertexTable[i];
          const float alpha = std::clamp((float(data[index[0]])-float(isovalue)) / (float(data[index[0]])-float(data[index[1]])),0.0f,1.0f);
          const Vec2 positionInSquare{vertexPosTable[index[0]] + ( vertexPosTable[index[1]]-vertexPosTable[index[0]]) * alpha};
          vertices.push_back((squareOffset + positionInSquare/Vec2{float(image.width), float(image.height)})*2.0f-Vec2(1.0f,1.0f));
        }
      }
      
      // ambiguous cases
      if (useAsymptoticDecider && (msCase == 5 || msCase == 10)) {
        const float decider{float(data[3]*data[1]-data[2]*data[0]) / float(data[1]+data[3]-data[2]-data[0])};
        if ((decider < isovalue && msCase == 5) || (decider > isovalue && msCase == 10)) {
          std::swap(vertices[vertices.size()-1],vertices[vertices.size()-3]);
        }
      }
    }
  }
}
