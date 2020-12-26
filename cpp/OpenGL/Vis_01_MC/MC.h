#pragma once

#include <vector>
#include "Volume.h"

struct Vertex {
  Vec3 position;
  Vec3 normal;
};

struct Isosurface {
  Isosurface(const Volume& volume, uint8_t isovalue);
  std::vector<Vertex> vertices;
};
