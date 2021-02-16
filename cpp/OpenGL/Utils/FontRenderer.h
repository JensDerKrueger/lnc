#pragma once

#include <string>
#include <vector>
#include <utility>

#include "bmp.h"
#include "Vec2.h"

struct CharPosition {
  char c;
  Vec2ui topLeft;
  Vec2ui bottomRight;
};

class FontRenderer {
public:
  FontRenderer(const std::string& imageFilename,
               const std::string& positionFilename);

  FontRenderer(const Image& fontImage,
               const std::string& positionFilename);

  FontRenderer(const Image& fontImage,
               const std::vector<CharPosition>& positions);

  Image render(const std::string& text);
  Image render(uint32_t number);
  
  static const std::vector<CharPosition> loadPositions(const std::string& positionFilename);

  std::string toCode(const std::string& varName);

private:
  Image fontImage;
  std::vector<CharPosition> positions;
  
  const CharPosition& findElement(char c);
};
