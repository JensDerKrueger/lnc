#pragma once

#include <string>
#include <Image.h>
#include <NetCommon.h>

enum class PayloadType {
  InvalidPayload = 0,
  BasicPayload = 1,
  MousePosPayload = 2,
  NewUserPayload = 3,
  LostUserPayload = 4,
  CanvasUpdatePayload = 5,
  InitPayload = 6
};

PayloadType identifyString(const std::string& s) {
  std::vector<std::string> l = Coder::decode(s);
  
  if (l.size() < 2) return PayloadType::InvalidPayload;
  if (l[0] != "painter") return PayloadType::InvalidPayload;
  
  int i;
  try {
    i = std::stoi( l[1] );
    if (i < 0 || i > int(PayloadType::InitPayload)) return PayloadType::InvalidPayload;
  } catch (const std::invalid_argument&) {
    return PayloadType::InvalidPayload;
  }
  
  return PayloadType(std::stoi( l[1] ));
}

struct BasicPayload {
  PayloadType pt;
  
  BasicPayload()
  {
    pt = PayloadType::BasicPayload;
  }
  
  std::string toString() {
    return Coder::encode({
      "painter",
      std::to_string(int(pt))
    });
  }
  
};

struct MousePosPayload : public BasicPayload {
  size_t userId;
  Vec2 mousePos;

  MousePosPayload(size_t userId, const Vec2& mousePos) :
  userId(userId),
  mousePos(mousePos)
  {
    pt = PayloadType::MousePosPayload;
  }

  std::string toString() {
    return Coder::encode({
      BasicPayload::toString(),
      std::to_string(userId),
      std::to_string(mousePos.x()),
      std::to_string(mousePos.y())
    });
  }

};

struct NewUserPayload : public BasicPayload {
  size_t userId;
  std::string name;
  Vec4 color;
  
  NewUserPayload(size_t userId, const std::string& name, const Vec4& color) :
  userId(userId),
  name(name),
  color(color)
  {
    pt = PayloadType::NewUserPayload;
  }
  
  std::string toString() {
    return Coder::encode({
      BasicPayload::toString(),
      std::to_string(userId),
      name,
      std::to_string(color.x()),
      std::to_string(color.y()),
      std::to_string(color.z()),
      std::to_string(color.w())
    });
  }

};

struct LostUserPayload : public BasicPayload {
  size_t userId;
  
  LostUserPayload(size_t userId) :
  userId(userId)
  {
    pt = PayloadType::LostUserPayload;
  }
  
  std::string toString() {
    return Coder::encode({
      BasicPayload::toString(),
      std::to_string(userId)
    });
  }

};

/*
struct CanvasUpdatePayload : public BasicPayload {
  CanvasUpdatePayload() {
    pt = PayloadType::CanvasUpdatePayload;
  }
};


struct InitPayload : public BasicPayload {

  static const string id{"InitPayload"};
};
*/
