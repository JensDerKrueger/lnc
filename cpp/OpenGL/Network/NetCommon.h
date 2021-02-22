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


struct Coder {
  
  static inline char DELIM = char(1);

  static std::string encode(const std::vector<std::string>& data) {
    std::string result;
    for (size_t i = 0;i<data.size();++i) {
      result += removeDelim(data[i]);
      result += DELIM;
    }
    return result;
  }

  static std::vector<std::string> decode(const std::string& input, size_t maxItems=0) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t delimPos = input.find(DELIM,start);
    
    if (maxItems == 0) {
      while (delimPos != std::string::npos) {
        result.push_back(input.substr(start, delimPos - start));
        start = delimPos + 1;
        delimPos = input.find(DELIM, start);
      }
    } else {
      while (delimPos != std::string::npos && result.size() <= maxItems) {
        result.push_back(input.substr(start, delimPos - start));
        start = delimPos + 1;
        delimPos = input.find(DELIM, start);
      }
    }
    result.push_back( input.substr(start) );
    
    return result;
  }

  static std::string removeDelim(std::string input) {
    size_t pos=0;
    while(pos<input.size()) {
      pos=input.find(DELIM,pos);
      if(pos==std::string::npos) break;
      input.replace(pos,1,"");
    }
    return input;
  }
};

std::string genHandshake(const std::string& iv, const std::string& key);
std::string getIVFromHandshake(const std::string& message, const std::string& key);
