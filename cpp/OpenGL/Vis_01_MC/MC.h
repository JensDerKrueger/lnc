#pragma once

#include <string>
#include <vector>

#include "Volume.h"

struct Vertex {
  Vec3 position;
  Vec3 normal;
};

class Isosurface {
public:
  Isosurface(const Volume& volume, uint8_t isovalue);
  std::vector<Vertex> vertices;
};
