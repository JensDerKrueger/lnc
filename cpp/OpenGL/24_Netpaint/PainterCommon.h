#pragma once

#include <string>
#include <NetCommon.h>

#include <Image.h>
#include <Vec2.h>
#include <Vec4.h>

constexpr uint32_t imageWidth = 400;
constexpr uint32_t imageHeight= 400;

struct MouseInfo {
  size_t id{0};
  std::string name{""};
  Vec4 color{0,0,0,0};
  Vec2 pos{0,0};
};

enum class PayloadType {
  InvalidPayload = 0,
  BasicPayload = 1,
  MousePosPayload = 2,
  NewUserPayload = 3,
  LostUserPayload = 4,
  CanvasUpdatePayload = 5,
  InitPayload = 6
};


class DSException : public std::exception {
  public:
  DSException(const std::string& whatStr) : whatStr(whatStr) {}
    virtual const char* what() const throw() {
      return whatStr.c_str();
    }
  private:
    std::string whatStr;
};

PayloadType identifyString(const std::string& s);

struct BasicPayload {
  PayloadType pt;
  uint32_t userID{0};

  BasicPayload(const std::string& message) {
    std::vector<std::string> token = Coder::decode(message);
    if (token.size() < 3) {
      throw DSException("BasicPayload message to short");
    }
    pt = PayloadType(std::stoi(token[1]));
    userID = std::stoi(token[2]);
  }
  
  BasicPayload()
  {
    pt = PayloadType::BasicPayload;
  }
  
  virtual std::string toString() {
    return Coder::encode({
      "painter",
      std::to_string(int(pt)),
      std::to_string(userID),
    }, false);
  }
  
};

struct MousePosPayload : public BasicPayload {
  Vec2 mousePos;

  MousePosPayload(const std::string& message) :
    BasicPayload(message)
  {
    std::vector<std::string> token = Coder::decode(message);
    if (token.size() < 5) {
      throw DSException("MousePosPayload message to short");
    }
    mousePos = Vec2(std::stof(token[3]), std::stof(token[4]));
  }
  
  MousePosPayload(const Vec2& mousePos) :
    mousePos(mousePos)
  {
    pt = PayloadType::MousePosPayload;
  }

  virtual std::string toString() override {
    return Coder::encode({
      BasicPayload::toString(),
      std::to_string(mousePos.x()),
      std::to_string(mousePos.y())
    }, false);
  }

};

struct NewUserPayload : public BasicPayload {
  std::string name;
  Vec4 color;
  
  NewUserPayload(const std::string& message) :
    BasicPayload(message)
  {
    std::vector<std::string> token = Coder::decode(message);
    if (token.size() < 8) {
      throw DSException("NewUserPayload message to short");
    }
    name = token[3];
    color = Vec4(std::stof(token[4]), std::stof(token[5]),std::stof(token[6]), std::stof(token[7]));
  }
  
  NewUserPayload(const std::string& name, const Vec4& color) :
    name(name),
    color(color)
  {
    pt = PayloadType::NewUserPayload;
  }
  
  virtual std::string toString() override {
    return Coder::encode({
      BasicPayload::toString(),
      name,
      std::to_string(color.x()),
      std::to_string(color.y()),
      std::to_string(color.z()),
      std::to_string(color.w())
    }, false);
  }

};

struct LostUserPayload : public BasicPayload {
  
  LostUserPayload(const std::string& message) :
    BasicPayload(message)
  {
  }
  
  LostUserPayload() {
    pt = PayloadType::LostUserPayload;
  }
};

struct CanvasUpdatePayload : public BasicPayload {
  Vec4 color;
  Vec2i pos;

  CanvasUpdatePayload(const std::string& message) :
    BasicPayload(message)
  {
    std::vector<std::string> token = Coder::decode(message);
    if (token.size() < 9) {
      throw DSException("CanvasUpdatePayload message to short");
    }
    color = Vec4(std::stof(token[3]), std::stof(token[4]), std::stof(token[5]), std::stof(token[6]));
    pos = Vec2i(std::stoi(token[7]), std::stoi(token[8]));
  }
  
  CanvasUpdatePayload(const Vec4& color, const Vec2i& pos) :
    color(color),
    pos(pos)
  {
    pt = PayloadType::CanvasUpdatePayload;
  }
  
  virtual std::string toString() override {
    return Coder::encode({
      BasicPayload::toString(),
      std::to_string(color.x()),
      std::to_string(color.y()),
      std::to_string(color.z()),
      std::to_string(color.w()),
      std::to_string(pos.x()),
      std::to_string(pos.y())
    }, false);
  }
};

struct InitPayload : public BasicPayload {
  Image image;
  std::vector<MouseInfo> mouseInfos;

  InitPayload(const std::string& message) :
    BasicPayload(message)
  {
    std::vector<std::string> token = Coder::decode(message);
    if (token.size() < 6) {
      throw DSException("CanvasUpdatePayload message to short (first check)");
    }
    
    size_t pos = 3;
    const uint32_t w = std::stoi(token[pos++]);
    const uint32_t h = std::stoi(token[pos++]);
    image = Image(w,h);
    if (token.size() < pos+image.data.size()) {
      throw DSException("CanvasUpdatePayload message to short (second check)");
    }
    for (size_t i = 0;i<image.data.size();++i) {
      image.data[i] = std::stoi(token[pos++]);
    }


    mouseInfos.resize(std::stoi(token[pos++]));
    if (token.size() < pos+mouseInfos.size()*8) {
      throw DSException("CanvasUpdatePayload message to short (third check)");
    }
    for (size_t i = 0;i<mouseInfos.size();++i) {
      mouseInfos[i].id = std::stoi(token[pos++]);
      mouseInfos[i].name = token[pos++];
      mouseInfos[i].color = Vec4{std::stof(token[pos]),std::stof(token[pos+1]),std::stof(token[pos+2]),std::stof(token[pos+3])};
      pos += 4;
      mouseInfos[i].pos = Vec2{std::stof(token[pos]),std::stof(token[pos+1])};
      pos += 2;
    }

    
  }
  
  InitPayload(const Image& image, const std::vector<MouseInfo>& mouseInfos) :
    image(image),
    mouseInfos(mouseInfos)
  {
    pt = PayloadType::InitPayload;
  }
  
  virtual std::string toString() override {
    std::vector<std::string> v{BasicPayload::toString()};
    
    v.push_back(std::to_string(image.width));
    v.push_back(std::to_string(image.height));
    for (size_t i = 0;i<image.data.size();++i) {
      v.push_back(std::to_string(image.data[i]));
    }

    v.push_back(std::to_string(mouseInfos.size()));
    for (size_t i = 0;i<mouseInfos.size();++i) {
      v.push_back(std::to_string(mouseInfos[i].id));
      v.push_back(mouseInfos[i].name);
      v.push_back(std::to_string(mouseInfos[i].color.x()));
      v.push_back(std::to_string(mouseInfos[i].color.y()));
      v.push_back(std::to_string(mouseInfos[i].color.z()));
      v.push_back(std::to_string(mouseInfos[i].color.w()));
      v.push_back(std::to_string(mouseInfos[i].pos.x()));
      v.push_back(std::to_string(mouseInfos[i].pos.y()));
    }
    
    return Coder::encode(v,false);
  }
};

