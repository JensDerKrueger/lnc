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
  static Image render(const std::string& text,
                      const Image& image,
                      const std::vector<CharPosition>& positions);
  static Image render(const std::string& text,
                      const std::string& imageFilename,
                      const std::string& positionFilename);
  static Image render(uint32_t number,
                      const std::string& imageFilename,
                      const std::string& positionFilename);
  
  static const std::vector<CharPosition> loadPositions(const std::string& positionFilename);

  static std::string assetsToCode(const std::string& varName,
                                  const Image& image,
                                  const std::vector<CharPosition>& positions);

  
private:
  static const CharPosition& findElement(char c, const std::vector<CharPosition>& positions);
};
