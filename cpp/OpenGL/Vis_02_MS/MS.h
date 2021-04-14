#pragma once

#include <vector>

#include <Image.h>
#include <Vec2.h>

struct Isoline {
  Isoline(const Image& image, uint8_t isovalue, bool useAsymptoticDecider);
  std::vector<Vec2> vertices;
};
