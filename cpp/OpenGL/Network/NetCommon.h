#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <memory>
#include <string>

#include "AES.h"
#include "Sockets.h"


struct Coder {
  
  static inline char DELIM = char(1);
  
  static std::string encode(const std::string& name, const std::string& value) {
    return removeZeroes(name) + DELIM + removeZeroes(value);
  }

  static std::pair<std::string, std::string> decode(const std::string& input) {
    const size_t delimPos = input.find(DELIM,0);
    return std::make_pair<std::string, std::string>(input.substr(0,delimPos),input.substr(delimPos+1));
  }

  static std::string removeZeroes(std::string input) {
    size_t pos=0;
    while(pos<input.size()) {
      pos=input.find(DELIM,pos);
      if(pos==std::string::npos) break;
      input.replace(pos,1,"");
    }
    return input;
  }
};
