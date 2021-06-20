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

bool isBigEndian(void);

template <typename T> T swapEndian(T u) {
  static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");
  union {
    T u;
    unsigned char u8[sizeof(T)];
  } source, dest;

  source.u = u;
  for (size_t k = 0; k < sizeof(T); k++)
    dest.u8[k] = source.u8[sizeof(T) - k - 1];
  return dest.u;
}


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
  uint16_t nextUint16();
  int16_t nextInt16();
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
  virtual void add(const char msg[]) = 0;
  virtual void add(const std::string& msg) = 0;
  virtual void add(const std::vector<std::string>& v) = 0;
  virtual void add(uint8_t i) = 0;
  virtual void add(int8_t i) = 0;
  virtual void add(uint16_t i) = 0;
  virtual void add(int16_t i) = 0;
  virtual void add(uint32_t i) = 0;
  virtual void add(int32_t i) = 0;
  virtual void add(uint64_t i) = 0;
  virtual void add(int64_t i) = 0;
  virtual void add(float f) = 0;
  virtual void add(double d) = 0;
  virtual void add(bool b) = 0;
  virtual void clear() = 0;
};

class StringEncoder : public Encoder {
public:
  StringEncoder(char delimititer = char(1));

  virtual void add(const char msg[]) override;
  virtual void add(const std::string& msg) override;
  virtual void add(const std::vector<std::string>& v) override;
  virtual void add(uint8_t i) override;
  virtual void add(int8_t i) override;
  virtual void add(uint16_t i) override;
  virtual void add(int16_t i) override;
  virtual void add(uint32_t i) override;
  virtual void add(int32_t i) override;
  virtual void add(uint64_t i) override;
  virtual void add(int64_t i) override;
  virtual void add(float f) override;
  virtual void add(double d) override;
  virtual void add(bool b) override;
  
  std::string getEncodedMessage() const {return message;}
  virtual void clear() override {message = "";}
  
private:
  char delimititer;
  std::string message;
  
  std::string removeDelim(std::string input) const;
};

class BinaryEncoder : public Encoder {
public:
  virtual void add(const char msg[]) override;
  virtual void add(const std::string& msg) override;
  virtual void add(const std::vector<std::string>& v) override;
  virtual void add(uint8_t i) override;
  virtual void add(int8_t i) override;
  virtual void add(uint16_t i) override;
  virtual void add(int16_t i) override;
  virtual void add(uint32_t i) override;
  virtual void add(int32_t i) override;
  virtual void add(uint64_t i) override;
  virtual void add(int64_t i) override;
  virtual void add(float f) override;
  virtual void add(double d) override;
  virtual void add(bool b) override;
  
  std::vector<uint8_t> getEncodedMessage() const {return message;}
  virtual void clear() override {message = {};}
  
private:
  std::vector<uint8_t> message;
};


class BinaryDecoder {
public:
  BinaryDecoder(const std::vector<uint8_t>& message);
  
  std::vector<std::string> nextStringVector();
  std::string nextString();
  uint8_t nextUint8();
  int8_t nextInt8();
  uint16_t nextUint16();
  int16_t nextInt16();
  uint32_t nextUint32();
  int32_t nextInt32();
  uint64_t nextUint64();
  int64_t nextInt64();
  float nextFloat();
  double nextDouble();
  bool nextBool();
  
private:
  std::vector<uint8_t> message;
  size_t pos;
};


std::string genHandshake(const std::string& iv, const std::string& key);
std::string getIVFromHandshake(const std::string& message, const std::string& key);
