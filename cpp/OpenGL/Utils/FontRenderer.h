#pragma once

#include <string>
#include <vector>
#include <map>
#include <utility>

#include "bmp.h"
#include "Vec2.h"
#include "GLTexture2D.h"
#include "GLArray.h"
#include "GLBuffer.h"

struct CharPosition {
  char c;
  Vec2ui topLeft;
  Vec2ui bottomRight;
};

struct CharTex {
  GLTexture2D tex;
  Mat4 scale;
  Mat4 trans;
  float width;
  float height;
};

enum class Alignment {
  Left,
  Right,
  Center
};

class FontEngine {
public:
  FontEngine();
  virtual ~FontEngine() {}
  void render(const std::string& text, float winAspect, float height, const Vec2& pos, Alignment a = Alignment::Center);
  void renderFixedWidth(const std::string& text, float winAspect, float width, const Vec2& pos, Alignment a = Alignment::Center);

  Vec2 getSize(const std::string& text, float winAspect, float height) const;
  Vec2 getSizeFixedWidth(const std::string& text, float winAspect, float width) const;

  std::string getAllCharsString() const;

  std::map<char,CharTex> chars;
  
  
private:
  GLProgram simpleProg;
  GLArray   simpleArray;
  GLBuffer  simpleVb;

};

class FontRenderer {
public:
  FontRenderer(const std::string& imageFilename,
               const std::string& positionFilename);

  FontRenderer(const Image& fontImage,
               const std::string& positionFilename);

  FontRenderer(const Image& fontImage,
               const std::vector<CharPosition>& positions);

  Image render(const std::string& text) const;
  Image render(uint32_t number) const;
  
  static const std::vector<CharPosition> loadPositions(const std::string& positionFilename);

  std::string toCode(const std::string& varName) const;

  std::shared_ptr<FontEngine> generateFontEngine() const;
  
private:
  Image fontImage;
  std::vector<CharPosition> positions;
  
  const CharPosition& findElement(char c) const;
};
