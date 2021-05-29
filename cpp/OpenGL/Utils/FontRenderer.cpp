#include <string>
#include <fstream>
#include <sstream>

#include "FontRenderer.h"

const CharPosition& FontRenderer::findElement(char c) const {
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

Image FontRenderer::render(uint32_t number) const {
  return render(std::to_string(number));
}

Image FontRenderer::render(const std::string& text) const {
  
  Vec2ui dims{0,0};
  for (char element : text) {
    const auto& pos = findElement(element);
    Vec2ui size = pos.bottomRight-pos.topLeft;
    dims = Vec2ui{dims.x+size.x, std::max(dims.y, size.y)};
  }
  
  Image result{dims.x,dims.y,fontImage.componentCount,
  std::vector<uint8_t>(dims.x*dims.y*fontImage.componentCount)};
  
  Vec2ui currentPos{0,0};
  for (char element : text) {
    const auto& pos = findElement(element);
    Vec2ui size = pos.bottomRight-pos.topLeft;
    BMP::blit(fontImage, pos.topLeft, pos.bottomRight, result, currentPos);
    currentPos = Vec2ui(currentPos.x+size.x, currentPos.y);
  }
  
  return result;
}

std::string FontRenderer::toCode(const std::string& varName) const {
  std::stringstream ss;
  ss << fontImage.toCode(varName+"Image") << "\nstd::vector<CharPosition> " << varName << "Pos{";
  
  for (size_t i = 0;i<positions.size();++i) {
    const CharPosition& p = positions[i];
    ss << "{" << int(p.c) << ", {" << p.topLeft.x << "," << p.topLeft.y << "},";
    ss << "{" << p.bottomRight.x << "," << p.bottomRight.y << "}}";
    
    if (i % 3 == 0) ss << "\n  ";
    if (i<positions.size()-1) ss << ",";
  }
  ss << "};\n";
  
  return ss.str();
}

std::shared_ptr<FontEngine> FontRenderer::generateFontEngine() const {
  std::shared_ptr<FontEngine> fe = std::make_shared<FontEngine>();
  
  uint32_t maxWidth  = 0;
  uint32_t maxHeight = 0;
  
  for (const CharPosition& c : positions) {
    const uint32_t width  = c.bottomRight.x-c.topLeft.x;
    const uint32_t height = c.bottomRight.y-c.topLeft.y;
    maxWidth = std::max(maxWidth, width);
    maxHeight = std::max(maxHeight, height);
  }
  
  for (const CharPosition& c : positions) {
    const Image i = render(std::string(1,c.c));
    const float w=i.width/float(maxWidth);
    const float h=i.height/float(maxHeight);
    
    const Mat4 s = Mat4::scaling(w,h,1.0f);
    const Mat4 t = Mat4::translation(w,h,0.0f);
    fe->chars[c.c] = CharTex{GLTexture2D(i),s,t,w,h};
    fe->chars[c.c].tex.setFilter(GL_LINEAR, GL_LINEAR);

    fe->sdChars[c.c] = CharTex{Grid2D(i).toSignedDistance(0.9f).toTexture(),s,t,w,h};
    fe->sdChars[c.c].tex.setFilter(GL_LINEAR, GL_LINEAR);
  }
  return fe;
}


FontEngine::FontEngine() :
  simpleProg{GLProgram::createFromString(
   "#version 410\n"
   "uniform mat4 MVP;\n"
   "layout (location = 0) in vec3 vPos;\n"
   "layout (location = 1) in vec2 vTexCoords;\n"
   "out vec4 color;\n"
   "out vec2 texCoords;\n"
   "void main() {\n"
   "    gl_Position = MVP * vec4(vPos, 1.0);\n"
   "    texCoords = vTexCoords;\n"
   "}\n",
   "#version 410\n"
   "uniform sampler2D raster;\n"
   "uniform vec4 globalColor;\n"
   "in vec2 texCoords;\n"
   "out vec4 FragColor;\n"
   "void main() {\n"
   "    FragColor = globalColor*texture(raster, texCoords);\n"
   "}\n")},
  simpleDistProg{GLProgram::createFromString(
   "#version 410\n"
   "uniform mat4 MVP;\n"
   "layout (location = 0) in vec3 vPos;\n"
   "layout (location = 1) in vec2 vTexCoords;\n"
   "out vec4 color;\n"
   "out vec2 texCoords;\n"
   "void main() {\n"
   "    gl_Position = MVP * vec4(vPos, 1.0);\n"
   "    texCoords = vTexCoords;\n"
   "}\n",
   "#version 410\n"
   "uniform sampler2D raster;\n"
   "uniform vec4 globalColor;\n"
   "in vec2 texCoords;\n"
   "out vec4 FragColor;\n"
   "void main() {\n"
   "    float dist = texture(raster, texCoords).r;\n"
   "    float val  = smoothstep(-3.0,1.0,dist);\n"
   "    FragColor  = globalColor*val;\n"
   "}\n")},
  simpleArray{},
  simpleVb{GL_ARRAY_BUFFER},
  renderAsSignedDistanceField{false}
{
  simpleArray.bind();
  std::vector<float> data = {
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,

    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f
  };
  simpleVb.setData(data,5,GL_STATIC_DRAW);
}

void FontEngine::render(const std::string& text, float winAspect,
                        float height, const Vec2& pos, Alignment a, const Vec4& color) {
  
  GLProgram& activeShader = (renderAsSignedDistanceField) ? simpleDistProg : simpleProg;
  std::map<char,CharTex>& activeFontMap = (renderAsSignedDistanceField) ? sdChars : chars;
  
  activeShader.enable();
  activeShader.setUniform("globalColor", color);
  simpleArray.bind();
  simpleArray.connectVertexAttrib(simpleVb, activeShader, "vPos", 3);
  simpleArray.connectVertexAttrib(simpleVb, activeShader, "vTexCoords", 2, 3);
  
  float totalWidth = 0;
  for (char c : text) {
    if (activeFontMap.find(c) == activeFontMap.end()) c = '_';
    totalWidth += activeFontMap.at(c).width;
  }
  
  Mat4 scale = Mat4::scaling(height/winAspect, height, 1.0f);
  Mat4 trans;
  switch (a) {
    case Alignment::Center :
      trans = Mat4::translation(pos.x-height*totalWidth/winAspect, pos.y, 0.0f);
      break;
    case Alignment::Right :
      trans = Mat4::translation(pos.x-2.0f*height*totalWidth/winAspect, pos.y, 0.0f);
      break;
    default :
      trans = Mat4::translation(pos.x, pos.y, 0.0f);
      break;
  }
  
  for (char c : text) {
    if (activeFontMap.find(c) == activeFontMap.end()) c = '_';
    activeShader.setUniform("MVP",
                            trans *
                            Mat4::translation(height*activeFontMap[c].width/winAspect,height*(activeFontMap[c].height-1.0f),0.0f) *
                            activeFontMap[c].scale *
                            scale);
    activeShader.setTexture("raster",activeFontMap[c].tex,0);
    GL(glDrawArrays(GL_TRIANGLES, 0, GLsizei(6)));
    trans = Mat4::translation(2.0f*height*activeFontMap[c].width/winAspect,0.0f,0.0f) * trans;
  }
}

Vec2 FontEngine::getSize(const std::string& text, float winAspect, float height) const {
  float totalWidth = 0;
  for (char c : text) {
    if (chars.find(c) == chars.end()) c = '_';
    totalWidth += chars.at(c).width;
  }
  return {height*totalWidth/winAspect, height};
}

void FontEngine::render(uint32_t number, float winAspect, float height, const Vec2& pos,
                        Alignment a, const Vec4& color) {
  render(std::to_string(number), winAspect, height, pos, a, color);
}

void FontEngine::renderFixedWidth(uint32_t number, float winAspect, float width, const Vec2& pos,
                                  Alignment a, const Vec4& color) {
  renderFixedWidth(std::to_string(number), winAspect, width, pos, a, color);
}

Vec2 FontEngine::getSize(uint32_t number, float winAspect, float height) const {
  return getSize(std::to_string(number), winAspect, height);
}

Vec2 FontEngine::getSizeFixedWidth(uint32_t number, float winAspect, float width) const {
  return getSizeFixedWidth(std::to_string(number), winAspect, width);
}

Vec2 FontEngine::getSizeFixedWidth(const std::string& text, float winAspect, float width) const {
  float totalWidth = 0;
  for (char c : text) {
    if (chars.find(c) == chars.end()) c = '_';
    totalWidth += chars.at(c).width;
  }
  return {width, width*winAspect/totalWidth};
}


void FontEngine::renderFixedWidth(const std::string& text, float winAspect, float width, const Vec2& pos, Alignment a, const Vec4& color) {
  simpleProg.enable();
  simpleProg.setUniform("globalColor", color);
  simpleArray.bind();
  simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vPos", 3);
  simpleArray.connectVertexAttrib(simpleVb, simpleProg, "vTexCoords", 2, 3);
  
  float totalWidth = 0;
  for (char c : text) {
    if (chars.find(c) == chars.end()) c = '_';
    totalWidth += chars.at(c).width;
  }
  
  Mat4 scale = Mat4::scaling(width/totalWidth, width/totalWidth*winAspect, 1.0f);
  Mat4 trans;
  switch (a) {
    case Alignment::Center :
      trans = Mat4::translation(pos.x-width, pos.y, 0.0f);
      break;
    case Alignment::Right :
      trans = Mat4::translation(pos.x-2.0f*width, pos.y, 0.0f);
      break;
    default :
      trans = Mat4::translation(pos.x, pos.y, 0.0f);
      break;
  }
  
  for (char c : text) {
    if (chars.find(c) == chars.end()) c = '_';
    simpleProg.setUniform("MVP", trans *
                          Mat4::translation(width*chars[c].width/totalWidth, width/totalWidth*winAspect*(chars[c].height-1.0f),0.0f) *
                          chars[c].scale *
                          scale);

    simpleProg.setTexture("raster",chars[c].tex,0);
    GL(glDrawArrays(GL_TRIANGLES, 0, GLsizei(6)));
    trans = Mat4::translation(2.0f* width*chars[c].width/totalWidth,0.0f,0.0f) * trans;
  }
}


std::string FontEngine::getAllCharsString() const {
  std::stringstream ss;
  for (const auto& c : chars) {
    ss << c.first;
  }
  return ss.str();
}
