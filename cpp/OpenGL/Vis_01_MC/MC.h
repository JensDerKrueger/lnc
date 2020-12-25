#pragma once

#include <string>
#include <vector>
#include <Volume.h>

struct Vertex {
  Vec3 position;
  Vec3 normal;
};

class Isosurface {
public:
  Isosurface(const Volume& v, uint8_t isovalue) {
    mc(v,isovalue);
  }
    
  std::vector<Vertex> vertices;
  
private:
  void mc(const Volume& v, uint8_t isovalue);
};
