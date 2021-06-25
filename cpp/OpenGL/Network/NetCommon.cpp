#include "NetCommon.h"

#include <limits>

bool isBigEndian(void) {
  union {
    int i;
    char c[sizeof(int32_t)];
  } tmp;
  tmp.i=0x1020;
  return tmp.c[0]!=0x20;
}


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

uint16_t Tokenizer::nextUint16() {
  try {
    const auto conversion = std::stoll(nextString());
    
    if (conversion < std::numeric_limits<uint16_t>::min() ||
        conversion > std::numeric_limits<uint16_t>::max() )
      throw MessageException("Value out of uint16_t range");
    
    return uint16_t(conversion);
  } catch (const std::invalid_argument& ) {
    throw MessageException("Value is not an uint16_t");
  } catch (const std::out_of_range& ) {
    throw MessageException("Value out of uint16_t range");
  }
}

int16_t Tokenizer::nextInt16() {
  try {
    const auto conversion = std::stoll(nextString());
    
    if (conversion < std::numeric_limits<int16_t>::lowest() ||
        conversion > std::numeric_limits<int16_t>::max() )
      throw MessageException("Value out of int16_t range");
    
    return int8_t(conversion);
  } catch (const std::invalid_argument& ) {
    throw MessageException("Value is not an int16_t");
  } catch (const std::out_of_range& ) {
    throw MessageException("Value out of int16_t range");
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


StringEncoder::StringEncoder(char delimititer) :
delimititer(delimititer)
{
}

void StringEncoder::add(const char msg[]) {
  return add(std::string(msg));
}

void StringEncoder::add(const std::string& msg) {
  message += removeDelim(msg) + delimititer;
}

void StringEncoder::add(const std::vector<std::string>& v) {
  for (const std::string& s : v) {
    add(s);
  }
}

void StringEncoder::add(uint8_t i) {
  message += std::to_string(i) + delimititer;
}

void StringEncoder::add(int8_t i) {
  message += std::to_string(i) + delimititer;
}

void StringEncoder::add(uint16_t i) {
  message += std::to_string(i) + delimititer;
}

void StringEncoder::add(int16_t i) {
  message += std::to_string(i) + delimititer;
}

void StringEncoder::add(uint32_t i) {
  message += std::to_string(i) + delimititer;
}

void StringEncoder::add(int32_t i) {
  message += std::to_string(i) + delimititer;
}

void StringEncoder::add(uint64_t i) {
  message += std::to_string(i) + delimititer;
}

void StringEncoder::add(int64_t i) {
  message += std::to_string(i) + delimititer;
}

void StringEncoder::add(float f) {
  message += std::to_string(f) + delimititer;
}

void StringEncoder::add(double d) {
  message += std::to_string(d) + delimititer;
}

void StringEncoder::add(bool b) {
  message += std::to_string(b) + delimititer;
}

std::string StringEncoder::removeDelim(std::string input) const {
  size_t pos=0;
  while(pos<input.size()) {
    pos=input.find(delimititer,pos);
    if(pos==std::string::npos) break;
    input.replace(pos,1,"");
  }
  return input;
}

void BinaryEncoder::add(const char msg[]) {
  add(std::string(msg));
}

void BinaryEncoder::add(const std::string& msg) {
  add(uint32_t(msg.length()));
  message.insert(message.end(), msg.begin(), msg.end());
}

void BinaryEncoder::add(const std::vector<std::string>& v) {
  add(uint32_t(v.size()));
  for (const std::string& s : v) {
    add(s);
  }
}

void BinaryEncoder::add(uint8_t i) {
  message.push_back(i);
}

void BinaryEncoder::add(int8_t i) {
  add(uint8_t(i));
}

void BinaryEncoder::add(uint16_t i) {
  i = (isBigEndian()) ? swapEndian(i) : i;
  
  add(uint8_t((i >> 8) & 0xFF));
  add(uint8_t((i >> 0) & 0xFF));
}

void BinaryEncoder::add(int16_t i) {
  add(uint16_t(i));
}

void BinaryEncoder::add(uint32_t i) {
  i = (isBigEndian()) ? swapEndian(i) : i;
  
  add(uint8_t((i >> 24) & 0xFF));
  add(uint8_t((i >> 16) & 0xFF));
  add(uint8_t((i >> 8) & 0xFF));
  add(uint8_t((i >> 0) & 0xFF));
}

void BinaryEncoder::add(int32_t i) {
  add(uint32_t(i));
}

void BinaryEncoder::add(uint64_t i) {
  i = (isBigEndian()) ? swapEndian(i) : i;
  
  add(uint8_t((i >> 56) & 0xFF));
  add(uint8_t((i >> 48) & 0xFF));
  add(uint8_t((i >> 40) & 0xFF));
  add(uint8_t((i >> 32) & 0xFF));
  add(uint8_t((i >> 24) & 0xFF));
  add(uint8_t((i >> 16) & 0xFF));
  add(uint8_t((i >> 8) & 0xFF));
  add(uint8_t((i >> 0) & 0xFF));
}

void BinaryEncoder::add(int64_t i) {
  add(uint64_t(i));
}

void BinaryEncoder::add(float f) {
  uint32_t* i = reinterpret_cast<uint32_t*>(&f);
  add(*i);
}

void BinaryEncoder::add(double d) {
  uint64_t* i = reinterpret_cast<uint64_t*>(&d);
  add(*i);
}

void BinaryEncoder::add(bool b) {
  add(uint8_t(b));
}

BinaryDecoder::BinaryDecoder(const std::vector<uint8_t>& message) : message(message), pos(0) {}
  
std::vector<std::string> BinaryDecoder::nextStringVector() {
  uint32_t l = nextUint32();
  std::vector<std::string> result(l);
  for (uint32_t i = 0;i<l;++i) {
    result[i] = nextString();
  }
  return result;
}

std::string BinaryDecoder::nextString() {
  uint32_t l = nextUint32();
  std::string result;
  for (uint32_t i = 0;i<l;++i) {
    result.push_back(char(nextUint8()));
  }
  return result;
}

uint8_t BinaryDecoder::nextUint8() {
  if (pos >= message.size())
    throw MessageException("Message too short");

  return message[pos++];
}

int8_t BinaryDecoder::nextInt8(){
  return int8_t(nextUint8());
}

uint16_t BinaryDecoder::nextUint16(){
  uint16_t a = nextUint8();
  uint16_t b = nextUint8();
  uint16_t r =  (a << 8) | b;
  return isBigEndian() ? swapEndian(r) : r;
}

int16_t BinaryDecoder::nextInt16() {
  return int16_t(nextUint16());
}

uint32_t BinaryDecoder::nextUint32() {
  uint32_t a = nextUint8();
  uint32_t b = nextUint8();
  uint32_t c = nextUint8();
  uint32_t d = nextUint8();
  uint32_t r =  (a << 24) | (b << 16) | (c << 8) | d;
  return isBigEndian() ? swapEndian(r) : r;
}

int32_t BinaryDecoder::nextInt32() {
  return int32_t(nextUint32());
}

uint64_t BinaryDecoder::nextUint64() {
  uint64_t a = nextUint8();
  uint64_t b = nextUint8();
  uint64_t c = nextUint8();
  uint64_t d = nextUint8();
  uint64_t e = nextUint8();
  uint64_t f = nextUint8();
  uint64_t g = nextUint8();
  uint64_t h = nextUint8();
  uint64_t r =  (a << 56) | (b << 48) | (c << 40) | (d << 32) | (e << 24) | (f << 16) | (g << 8) | h;
  return isBigEndian() ? swapEndian(r) : r;
}

int64_t BinaryDecoder::nextInt64() {
  return int64_t(nextUint64());
}

float BinaryDecoder::nextFloat()  {
  uint32_t i = nextUint32();
  float* f = reinterpret_cast<float*>(&i);
  return *f;
}

double BinaryDecoder::nextDouble() {
  uint64_t i = nextUint64();
  double* d = reinterpret_cast<double*>(&i);
  return *d;
}

bool BinaryDecoder::nextBool() {
  return nextUint8();
}
