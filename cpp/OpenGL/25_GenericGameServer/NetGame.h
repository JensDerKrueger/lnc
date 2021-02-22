#pragma once

#include <sstream>
#include <string>
#include <memory>
#include <vector>
#include <exception>

#include <NetCommon.h>

constexpr uint16_t serverPort = 11003;

enum class MessageType {
  InvalidMessage = 0,
  BasicMessage = 1,
  ConnectMessage = 2,
  LostUserMessage = 3,
  PairedMessage = 4,
  GameMessage = 5
};

MessageType identifyString(const std::string& s);

struct BasicMessage {
  MessageType pt;
  uint32_t userID{0};
  Tokenizer tokenizer;

  BasicMessage(const std::string& message) :
    tokenizer{message}
  {
    if (tokenizer.nextString() != "game") throw MessageException("Invalid message");
    pt = MessageType(tokenizer.nextUint32());
    userID = tokenizer.nextUint32();
    pt = MessageType::BasicMessage;
  }
  
  BasicMessage() :
    tokenizer{""}
  {
    pt = MessageType::BasicMessage;
  }
  
  virtual ~BasicMessage() {}
  
  virtual std::string toString() {
    return Coder::encode({
      "game",
      std::to_string(int(pt)),
      std::to_string(userID),
    });
  }
};

struct LostUserMessage : public BasicMessage {
  
  LostUserMessage(const std::string& message) :
    BasicMessage(message)
  {
    pt = MessageType::LostUserMessage;
  }
  
  LostUserMessage() {
    pt = MessageType::LostUserMessage;
  }
  
  virtual ~LostUserMessage() {}
};


struct PairedMessage : public BasicMessage {
  
  PairedMessage(const std::string& message) :
    BasicMessage(message)
  {
    pt = MessageType::PairedMessage;
  }
  
  PairedMessage() {
    pt = MessageType::PairedMessage;
  }
  
  virtual ~PairedMessage() {}
};


struct ConnectMessage : public BasicMessage {
  std::string name;
  uint32_t gameID;
  uint32_t level;
  
  ConnectMessage(const std::string& message) :
    BasicMessage(message)
  {
    name = tokenizer.nextString();
    gameID = tokenizer.nextUint32();
    level = tokenizer.nextUint32();
    pt = MessageType::ConnectMessage;
  }
  
  ConnectMessage(const std::string& name, uint32_t gameID, uint32_t level) :
    name(name),
    gameID(gameID),
    level(level)
  {
    pt = MessageType::ConnectMessage;
  }
  
  virtual ~ConnectMessage() {}
  
  virtual std::string toString() override {
    return BasicMessage::toString() + Coder::encode({
      name,
      std::to_string(gameID),
      std::to_string(level),
    });
  }
};


struct GameMessage : public BasicMessage {
  std::string payload;
  
  GameMessage(const std::string& message) :
    BasicMessage(message)
  {
    payload = tokenizer.nextString();
    pt = MessageType::GameMessage;
  }
  
  GameMessage() :
    payload{}
  {
    pt = MessageType::GameMessage;
  }
  
  virtual ~GameMessage() {}
  
  virtual std::string toString() override {
    return BasicMessage::toString() + Coder::encode({payload});
  }
};
