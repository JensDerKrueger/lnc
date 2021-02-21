#pragma once

#include <sstream>
#include <string>
#include <memory>
#include <vector>
#include <exception>

#include <NetCommon.h>

constexpr uint16_t serverPort = 11003;

class MessageException : public std::exception {
  public:
  MessageException(const std::string& whatStr) : whatStr(whatStr) {}
    virtual const char* what() const throw() {
      return whatStr.c_str();
    }
  private:
    std::string whatStr;
};

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
  size_t startToken;
  std::vector<std::string> token;

  BasicMessage(const std::string& message) {
    token = Coder::decode(message);
    if (token.size() < 3) {
      throw MessageException("BasicMessage message to short");
    }
    pt = MessageType(std::stoi(token[1]));
    userID = std::stoi(token[2]);
    startToken = 3;
    pt = MessageType::BasicMessage;
  }
  
  BasicMessage() {
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
    if (token.size() < startToken + 3) {
      std::stringstream ss;
      ss << "ConnectMessage message to short. Expected at least " << startToken + 3 << " elements but received only " << token.size() << ".";
      throw MessageException(ss.str());
    }
    name = token[startToken+0];
    gameID = std::stoi(token[startToken+1]);
    level = std::stoi(token[startToken+2]);
    startToken += 3;
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
    return Coder::append(BasicMessage::toString(), {
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
    if (token.size() < startToken + 1) {
      std::stringstream ss;
      ss << "GameMessage message to short. Expected at least " << startToken + 1 << " elements but received only " << token.size() << ".";
      throw MessageException(ss.str());
    }
    payload = token[startToken];
    startToken += 1;
    pt = MessageType::GameMessage;
  }
  
  GameMessage() :
  payload{}
  {
    pt = MessageType::GameMessage;
  }
  
  virtual ~GameMessage() {}
  
  virtual std::string toString() override {
    return Coder::append(BasicMessage::toString(), {
      payload,
    });
  }
};
