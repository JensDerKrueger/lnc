#include <string>
#include <fstream>
#include <sstream>

#include "FontRenderer.h"

const CharPosition& FontRenderer::findElement(char c) {
  for (size_t i = 0;i<positions.size();++i) {
    if (positions[i].c == c) return positions[i];
  }
  return positions[0];
}

const std::vector<CharPosition> FontRenderer::loadPositions(const std::string& positionFilename) {
  std::vector<CharPosition> positions;
  std::ifstream posfile (positionFilename);
  std::string line;
  if (posfile.is_open()) {
    while (getline(posfile,line) ) {
      std::vector<std::string> vals;
      std::stringstream tokenizer(line);
      std::string token;
      while(getline(tokenizer, token, ' ')) {
        vals.push_back(token);
      }
    
      if (vals.size() == 5) {
        positions.push_back({vals[0][0],
                             {uint32_t(stoi(vals[1])),uint32_t(stoi(vals[2]))},
                             {uint32_t(stoi(vals[3])),uint32_t(stoi(vals[4]))}});
      } else if (vals.size() == 6) {
        positions.push_back({' ',
                             {uint32_t(stoi(vals[2])),uint32_t(stoi(vals[3]))},
                             {uint32_t(stoi(vals[4])),uint32_t(stoi(vals[5]))}});
      }
    }
    posfile.close();
  }
  return positions;
}

FontRenderer::FontRenderer(const std::string& imageFilename,
                           const std::string& positionFilename) :
  FontRenderer(BMP::load(imageFilename), loadPositions(positionFilename))
{
}

FontRenderer::FontRenderer(const Image& fontImage,
                           const std::string& positionFilename) :
  FontRenderer(fontImage, loadPositions(positionFilename))
{
}

FontRenderer::FontRenderer(const Image& fontImage,
                           const std::vector<CharPosition>& positions) :
fontImage(fontImage),
positions(positions)
{
  if (fontImage.componentCount == 3) this->fontImage.generateAlphaFromLuminance();
}

Image FontRenderer::render(uint32_t number) {
  return render(std::to_string(number));
}

Image FontRenderer::render(const std::string& text) {
  
  Vec2ui dims{0,0};
  for (char element : text) {
    const auto& pos = findElement(element);
    Vec2ui size = pos.bottomRight-pos.topLeft;
    dims = Vec2ui{dims.x()+size.x(), std::max(dims.y(), size.y())};
  }
  
  Image result{dims.x(),dims.y(),fontImage.componentCount,
  std::vector<uint8_t>(dims.x()*dims.y()*fontImage.componentCount)};
  
  Vec2ui currentPos{0,0};
  for (char element : text) {
    const auto& pos = findElement(element);
    Vec2ui size = pos.bottomRight-pos.topLeft;
    BMP::blit(fontImage, pos.topLeft, pos.bottomRight, result, currentPos);
    currentPos = Vec2ui(currentPos.x()+size.x(), currentPos.y());
  }
  
  return result;
}

std::string FontRenderer::toCode(const std::string& varName) {
  std::stringstream ss;
  ss << fontImage.toCode(varName+"Image") << "\nstd::vector<CharPosition> " << varName << "Pos{";
  
  for (size_t i = 0;i<positions.size();++i) {
    const CharPosition& p = positions[i];
    ss << "{" << int(p.c) << ", {" << p.topLeft.x() << "," << p.topLeft.y() << "},";
    ss << "{" << p.bottomRight.x() << "," << p.bottomRight.y() << "}}";
    
    if (i % 3 == 0) ss << "\n  ";
    if (i<positions.size()-1) ss << ",";
  }
  ss << "};\n";
  
  return ss.str();
}
