#include "NetCommon.h"

#include <limits>

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


Tokenizer::Tokenizer(const std::string& message, char delimititer) :
delimititer(delimititer),
message(message)
{
}
  
std::string Tokenizer::nextString() {
  size_t delimPos = message.find(delimititer,currentIndex);
  
  if (delimPos == std::string::npos)
      throw MessageException("Message too short");
  
  const std::string result = message.substr(currentIndex, delimPos - currentIndex);
  currentIndex = delimPos + 1;
  return result;
}

uint8_t Tokenizer::nextUint8() {
  try {
    const auto conversion = std::stoll(nextString());
    
    if (conversion < std::numeric_limits<uint8_t>::min() ||
        conversion > std::numeric_limits<uint8_t>::max() )
      throw MessageException("Value out of uint8_t range");
    
    return uint8_t(conversion);
  } catch (const std::invalid_argument& ) {
    throw MessageException("Value is not an uint8_t");
  } catch (const std::out_of_range& ) {
    throw MessageException("Value out of uint8_t range");
  }
}

int8_t Tokenizer::nextInt8() {
  try {
    const auto conversion = std::stoll(nextString());
    
    if (conversion < std::numeric_limits<int8_t>::lowest() ||
        conversion > std::numeric_limits<int8_t>::max() )
      throw MessageException("Value out of int8_t range");
    
    return int8_t(conversion);
  } catch (const std::invalid_argument& ) {
    throw MessageException("Value is not an int8_t");
  } catch (const std::out_of_range& ) {
    throw MessageException("Value out of int8_t range");
  }
}


uint32_t Tokenizer::nextUint32() {
  try {
    const auto conversion = std::stoll(nextString());
    
    if (conversion < std::numeric_limits<uint32_t>::min() ||
        conversion > std::numeric_limits<uint32_t>::max() )
      throw MessageException("Value out of uint32_t range");
    
    return uint32_t(conversion);
  } catch (const std::invalid_argument& ) {
    throw MessageException("Value is not an uint32_t");
  } catch (const std::out_of_range& ) {
    throw MessageException("Value out of uint32_t range");
  }
}

int32_t Tokenizer::nextInt32() {
  try {
    const auto conversion = std::stoll(nextString());
    
    if (conversion < std::numeric_limits<int32_t>::lowest() ||
        conversion > std::numeric_limits<int32_t>::max() )
      throw MessageException("Value out of int32_t range");
    
    return int32_t(conversion);
  } catch (const std::invalid_argument& ) {
    throw MessageException("Value is not an int32_t");
  } catch (const std::out_of_range& ) {
    throw MessageException("Value out of int32_t range");
  }
}

uint64_t Tokenizer::nextUint64() {
  try {
    const auto conversion = std::stoll(nextString());
    
    if (conversion < 0)
      throw MessageException("Value out of uint64_t range");
    
    return uint64_t(conversion);
  } catch (const std::invalid_argument& ) {
    throw MessageException("Value is not an int64_t");
  } catch (const std::out_of_range& ) {
    throw MessageException("Value out of uint64_t range");
  }
}

int64_t Tokenizer::nextInt64() {
  try {
    const auto conversion = std::stoll(nextString());
    
    if (conversion < std::numeric_limits<int64_t>::lowest() ||
        conversion > std::numeric_limits<int64_t>::max() )
      throw MessageException("Value out of int64_t range");
    
    return int64_t(conversion);
  } catch (const std::invalid_argument& ) {
    throw MessageException("Value is not an int64_t");
  } catch (const std::out_of_range& ) {
    throw MessageException("Value out of int64_t range");
  }
}


float Tokenizer::nextFloat() {
  try {
    return std::stof(nextString());
  } catch (const std::invalid_argument& ) {
    throw MessageException("Value is not a float");
  } catch (const std::out_of_range& ) {
    throw MessageException("Value out of float range");
  }
}

double Tokenizer::nextDouble() {
  try {
    return std::stod(nextString());
  } catch (const std::invalid_argument& ) {
    throw MessageException("Value is not a double");
  } catch (const std::out_of_range& ) {
    throw MessageException("Value out of double range");
  }
}

bool Tokenizer::nextBool() {
  return nextUint32() != 0;
}


Encoder::Encoder(char delimititer) :
delimititer(delimititer)
{
}

void Encoder::add(const char msg[]) {
  return add(std::string(msg));
}

void Encoder::add(const std::string& msg) {
  message += removeDelim(msg) + delimititer;
}

void Encoder::add(const std::vector<std::string>& v) {
  for (const std::string& s : v) {
    add(s);
  }
}

void Encoder::add(uint8_t i) {
  message += std::to_string(i) + delimititer;
}

void Encoder::add(int8_t i) {
  message += std::to_string(i) + delimititer;
}

void Encoder::add(uint32_t i) {
  message += std::to_string(i) + delimititer;
}

void Encoder::add(int32_t i) {
  message += std::to_string(i) + delimititer;
}

void Encoder::add(uint64_t i) {
  message += std::to_string(i) + delimititer;
}

void Encoder::add(int64_t i) {
  message += std::to_string(i) + delimititer;
}

void Encoder::add(float f) {
  message += std::to_string(f) + delimititer;
}

void Encoder::add(double d) {
  message += std::to_string(d) + delimititer;
}

void Encoder::add(bool b) {
  message += std::to_string(b) + delimititer;
}
std::string Encoder::removeDelim(std::string input) const {
  size_t pos=0;
  while(pos<input.size()) {
    pos=input.find(delimititer,pos);
    if(pos==std::string::npos) break;
    input.replace(pos,1,"");
  }
  return input;
}
