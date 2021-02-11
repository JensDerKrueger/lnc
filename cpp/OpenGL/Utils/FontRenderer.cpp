#include <string>
#include <fstream>
#include <sstream>

#include "FontRenderer.h"

const CharPosition& FontRenderer::findElement(char c, const std::vector<CharPosition>& positions) {
  for (size_t i = 0;i<positions.size();++i) {
    if (positions[i].c == c) return positions[i];
  }
  return positions[0];
}

Image FontRenderer::render(uint32_t number,
                           const std::string& imageFilename,
                           const std::string& positionFilename) {
  return render(std::to_string(number), imageFilename, positionFilename);
}


Image FontRenderer::render(const std::string& text,
                           const std::string& imageFilename,
                           const std::string& positionFilename) {

  Image numberSource = BMP::load(imageFilename);
  
  std::vector<CharPosition> positions;
  std::ifstream posfile (positionFilename);
  std::string line;
  if (posfile.is_open()) {
      while ( getline (posfile,line) ) {
          std::vector<std::string> vals;
          std::stringstream tokenizer(line);
          std::string token;
          while(getline(tokenizer, token, ' ')) {
              vals.push_back(token);
          }
          if (vals.size() == 5)
            positions.push_back({vals[0][0],
                                 {uint32_t(stoi(vals[1])),uint32_t(stoi(vals[2]))},
                                 {uint32_t(stoi(vals[3])),uint32_t(stoi(vals[4]))}});
      }
      posfile.close();
  }
  
  Vec2ui dims{0,0};
  for (char element : text) {
      const auto& pos = findElement(element, positions);
      Vec2ui size = pos.bottomRight-pos.topLeft;
      dims = Vec2ui{dims.x()+size.x(), std::max(dims.y(), size.y())};
  }
  
  Image result{dims.x(),dims.y(),numberSource.componentCount,
      std::vector<uint8_t>(dims.x()*dims.y()*numberSource.componentCount)};
  
  Vec2ui currentPos{0,0};
  for (char element : text) {
    const auto& pos = findElement(element, positions);
    Vec2ui size = pos.bottomRight-pos.topLeft;
    BMP::blit(numberSource, pos.topLeft, pos.bottomRight, result, currentPos);
    currentPos = Vec2ui(currentPos.x()+size.x(), currentPos.y());
  }
  
  return result;
}
