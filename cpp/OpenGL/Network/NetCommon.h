#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <random>

#include "AES.h"
#include "Sockets.h"

class MessageException : public std::exception {
  public:
  MessageException(const std::string& whatStr) : whatStr(whatStr) {}
    virtual const char* what() const throw() {
      return whatStr.c_str();
    }
  private:
    std::string whatStr;
};

class Tokenizer {
public:
  Tokenizer(const std::string& message, char delimititer = char(1));
  
  std::string nextString();
  uint8_t nextUint8();
  int8_t nextInt8();
  uint32_t nextUint32();
  int32_t nextInt32();
  uint64_t nextUint64();
  int64_t nextInt64();
  float nextFloat();
  double nextDouble();
  bool nextBool();
  
private:
  char delimititer;
  const std::string message;
  size_t currentIndex{0};
};

class Encoder {
public:
  Encoder(char delimititer = char(1));

  void add(const char msg[]);
  void add(const std::string& msg);
  void add(const std::vector<std::string>& v);
  void add(uint8_t i);
  void add(int8_t i);
  void add(uint32_t i);
  void add(int32_t i);
  void add(uint64_t i);
  void add(int64_t i);
  void add(float f);
  void add(double d);
  void add(bool b);
  
  std::string getEncodedMessage() const {return message;}
  void clear() {message = "";}
  
private:
  char delimititer;
  std::string message;
  
  std::string removeDelim(std::string input) const;
};

std::string genHandshake(const std::string& iv, const std::string& key);
std::string getIVFromHandshake(const std::string& message, const std::string& key);
