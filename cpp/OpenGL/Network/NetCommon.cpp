#include "NetCommon.h"

std::string genHandshake(const std::string& iv, const std::string& key) {
  std::string message{key+iv};
  
  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 generator((unsigned int)seed);  // mt19937 is a standard mersenne_twister_engine
  
  const size_t randomLength = generator()%200;
  for (size_t i = 0;i<randomLength;++i) {
    message = message + "X";
  }

  return message;
}


std::string getIVFromHandshake(const std::string& message, const std::string& key) {
  if (message.size() < 32) {
    return "";
  }
  
  if (message.substr(0,16) != key) {
    return "";
  }

  return message.substr(16,16);
}
