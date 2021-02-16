#pragma once

#include <string>
#include <NetCommon.h>

#include <Image.h>
#include <Vec2.h>
#include <Vec4.h>

constexpr uint32_t imageWidth = 400;
constexpr uint32_t imageHeight= 400;
constexpr uint16_t serverPort = 11002;

class ClientInfo {
public:
  uint32_t id{0};
  std::string name{""};
  Vec4 color{0,0,0,0};
  Vec2 pos{0,0};
  
  ClientInfo() {}
  
  ClientInfo(uint32_t id, const std::string& name, const Vec4& color, const Vec2 pos) :
    id{id},
    name{cleanupName(name)},
    color{color},
    pos{pos}
  {}

  virtual ~ClientInfo() {}
  
  static std::string cleanupName(const std::string& name) {
    std::string cName{name};
    for (size_t i = 0;i<name.length();++i) {
      cName[i] = cleanupChar(name[i]);
    }
    return cName;
  }

private:
  static char cleanupChar(char c) {
    std::string validChars{"01234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ(),._ ;:"};
    if (validChars.find(c) != std::string::npos) return c; else return '_';
  }

};

class ClientInfoServerSide : public ClientInfo {
public:
  bool fastCursorUpdates;
  
  ClientInfoServerSide(uint32_t id, const std::string& name, const Vec4& color,
                       const Vec2 pos, bool fastCursorUpdates) :
  ClientInfo(id, name, color, pos),
  fastCursorUpdates(fastCursorUpdates)
  {
  }

  virtual ~ClientInfoServerSide() {}
};

class ClientInfoClientSide : public ClientInfo {
public:
  Image image;
  
  ClientInfoClientSide(uint32_t id, const std::string& name, const Vec4& color,
                       const Vec2 pos, const Image& image) :
  ClientInfo(id, name, color, pos),
  image(image)
  {
  }
  
  virtual ~ClientInfoClientSide() {}
};

enum class PayloadType {
  InvalidPayload = 0,
  BasicPayload = 1,
  MousePosPayload = 2,
  NewUserPayload = 3,
  LostUserPayload = 4,
  CanvasUpdatePayload = 5,
  InitPayload = 6,
  ConnectPayload = 7
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
  size_t startToken;
  std::vector<std::string> token;

  BasicPayload(const std::string& message) {
    token = Coder::decode(message);
    if (token.size() < 3) {
      throw DSException("BasicPayload message to short");
    }
    pt = PayloadType(std::stoi(token[1]));
    userID = std::stoi(token[2]);
    startToken = 3;
    pt = PayloadType::BasicPayload;
  }
  
  BasicPayload() {
    pt = PayloadType::BasicPayload;
  }
  
  virtual ~BasicPayload() {}
  
  virtual std::string toString() {
    return Coder::encode({
      "painter",
      std::to_string(int(pt)),
      std::to_string(userID),
    });
  }
  
};

struct MousePosPayload : public BasicPayload {
  Vec2 mousePos;

  MousePosPayload(const std::string& message) :
    BasicPayload(message)
  {
    if (token.size() < startToken + 2) {
      throw DSException("MousePosPayload message to short");
    }
    mousePos = Vec2(std::stof(token[startToken]), std::stof(token[startToken+1]));
    startToken += 2;
    pt = PayloadType::MousePosPayload;
  }
  
  MousePosPayload(const Vec2& mousePos) :
    mousePos(mousePos)
  {
    pt = PayloadType::MousePosPayload;
  }
  
  virtual ~MousePosPayload() {}

  virtual std::string toString() override {
    return Coder::append(BasicPayload::toString(), {std::to_string(mousePos.x()),std::to_string(mousePos.y())});
  }

};

struct NewUserPayload : public BasicPayload {
  std::string name;
  Vec4 color;
  
  NewUserPayload(const std::string& message) :
    BasicPayload(message)
  {
    if (token.size() < startToken + 5) {
      std::stringstream ss;
      ss << "NewUserPayload message to short. Expected at least " << startToken + 5 << " elements but received only " << token.size() << ".";
      throw DSException(ss.str());
    }
    name = token[startToken];
    color = Vec4(std::stof(token[startToken+1]), std::stof(token[startToken+2]),std::stof(token[startToken+3]), std::stof(token[startToken+4]));
    startToken += 5;
    pt = PayloadType::NewUserPayload;
  }
  
  NewUserPayload(const std::string& name, const Vec4& color) :
    name(name),
    color(color)
  {
    pt = PayloadType::NewUserPayload;
  }
  
  virtual ~NewUserPayload() {}
  
  virtual std::string toString() override {
    return Coder::append(BasicPayload::toString(), {
      name,
      std::to_string(color.x()),
      std::to_string(color.y()),
      std::to_string(color.z()),
      std::to_string(color.w())
    });
  }
};


struct ConnectPayload : public NewUserPayload {
  bool fastCursorUpdates;
  
  ConnectPayload(const std::string& message) :
    NewUserPayload(message)
  {
    if (token.size() < startToken + 1) {
      std::stringstream ss;
      ss << "ConnectPayload message to short. Expected at least " << startToken + 1 << " elements but received only " << token.size() << ".";
      throw DSException(ss.str());
    }
    fastCursorUpdates = std::stoi(token[startToken]);
    startToken += 1;
    pt = PayloadType::ConnectPayload;
  }
  
  ConnectPayload(const std::string& name, const Vec4& color, bool fastCursorUpdates) :
  NewUserPayload(name, color),
  fastCursorUpdates(fastCursorUpdates)
  {
    pt = PayloadType::ConnectPayload;
  }
  
  virtual ~ConnectPayload() {}
  
  virtual std::string toString() override {
    return Coder::append(NewUserPayload::toString(), {
      fastCursorUpdates ? "1" : "0"
    });
  }

};

struct LostUserPayload : public BasicPayload {
  
  LostUserPayload(const std::string& message) :
    BasicPayload(message)
  {
    pt = PayloadType::LostUserPayload;
  }
  
  LostUserPayload() {
    pt = PayloadType::LostUserPayload;
  }
  
  virtual ~LostUserPayload() {}
};

struct CanvasUpdatePayload : public BasicPayload {
  Vec4 color;
  Vec2i pos;

  CanvasUpdatePayload(const std::string& message) :
    BasicPayload(message)
  {
    if (token.size() < startToken + 6) {
      std::stringstream ss;
      ss << "CanvasUpdatePayload message to short. Expected at least " << startToken + 6 << " elements but received only " << token.size() << ".";
      throw DSException(ss.str());
    }
    color = Vec4(std::stof(token[startToken]), std::stof(token[startToken+1]), std::stof(token[startToken+2]), std::stof(token[startToken+3]));
    pos = Vec2i(std::stoi(token[startToken+4]), std::stoi(token[startToken+5]));
    
    startToken += 6;
    pt = PayloadType::CanvasUpdatePayload;
  }
  
  CanvasUpdatePayload(const Vec4& color, const Vec2i& pos) :
    color(color),
    pos(pos)
  {
    pt = PayloadType::CanvasUpdatePayload;
  }
  
  virtual ~CanvasUpdatePayload() {}
  
  virtual std::string toString() override {
    return Coder::append(BasicPayload::toString(), {
      std::to_string(color.x()),
      std::to_string(color.y()),
      std::to_string(color.z()),
      std::to_string(color.w()),
      std::to_string(pos.x()),
      std::to_string(pos.y())
    });
  }
};

struct InitPayload : public BasicPayload {
  Image image;
  std::vector<ClientInfo> clientInfos;

  InitPayload(const std::string& message) :
    BasicPayload(message)
  {
    if (token.size() < startToken + 3) {
      std::stringstream ss;
      ss << "InitPayload message to short (first check). Expected at least " << startToken + 3 << " elements but received only " << token.size() << ".";
      throw DSException(ss.str());
    }
    
    const uint32_t w = std::stoi(token[startToken++]);
    const uint32_t h = std::stoi(token[startToken++]);
    image = Image(w,h);
    if (token.size() < startToken+image.data.size()) {
      std::stringstream ss;
      ss << "InitPayload message to short (second check). Expected at least " << startToken + image.data.size() << " elements but received only " << token.size() << ".";
      throw DSException(ss.str());
    }
    for (size_t i = 0;i<image.data.size();++i) {
      image.data[i] = std::stoi(token[startToken++]);
    }

    clientInfos.resize(std::stoi(token[startToken++]));
    if (token.size() < startToken+clientInfos.size()*8) {
      std::stringstream ss;
      ss << "InitPayload message to short (third check). Expected at least " << (startToken+clientInfos.size()*8) << " elements but received only " << token.size() << ".";
      throw DSException(ss.str());
    }
    for (size_t i = 0;i<clientInfos.size();++i) {
      clientInfos[i].id = std::stoi(token[startToken++]);
      clientInfos[i].name = token[startToken++];
      clientInfos[i].color = Vec4{std::stof(token[startToken]),std::stof(token[startToken+1]),std::stof(token[startToken+2]),std::stof(token[startToken+3])};
      startToken += 4;
      clientInfos[i].pos = Vec2{std::stof(token[startToken]),std::stof(token[startToken+1])};
      startToken += 2;
    }
    pt = PayloadType::InitPayload;
  }

  InitPayload(const Image& image, const std::vector<ClientInfoServerSide>& clientInfosSS) :
    image(image)
  {
    clientInfos.resize(clientInfosSS.size());
    for (size_t i = 0;i<clientInfos.size();++i) {
      clientInfos[i].id = clientInfosSS[i].id;
      clientInfos[i].name = clientInfosSS[i].name;
      clientInfos[i].color = clientInfosSS[i].color;
      clientInfos[i].pos = clientInfosSS[i].pos;
    }
    pt = PayloadType::InitPayload;
  }
  
  virtual ~InitPayload() {}
  
  virtual std::string toString() override {
    std::vector<std::string> v;
    
    v.push_back(std::to_string(image.width));
    v.push_back(std::to_string(image.height));
    for (size_t i = 0;i<image.data.size();++i) {
      v.push_back(std::to_string(image.data[i]));
    }

    v.push_back(std::to_string(clientInfos.size()));
    for (size_t i = 0;i<clientInfos.size();++i) {
      v.push_back(std::to_string(clientInfos[i].id));
      v.push_back(clientInfos[i].name);
      v.push_back(std::to_string(clientInfos[i].color.x()));
      v.push_back(std::to_string(clientInfos[i].color.y()));
      v.push_back(std::to_string(clientInfos[i].color.z()));
      v.push_back(std::to_string(clientInfos[i].color.w()));
      v.push_back(std::to_string(clientInfos[i].pos.x()));
      v.push_back(std::to_string(clientInfos[i].pos.y()));
    }
    
    return Coder::append(BasicPayload::toString(), v);
  }
};

