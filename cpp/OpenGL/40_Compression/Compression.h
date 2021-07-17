#pragma once

#include <string>
#include <vector>

namespace Compression {

  class BitBuffer {
  public:
    BitBuffer() {}
    BitBuffer(const std::vector<char>& data) :
      data{data}
    {
    }
    
    const std::vector<char>& getData() const {
      return data;
    }
    
    
  protected:
    std::vector<char> data;
  };

  class OutputBitBuffer : public BitBuffer {
  public:
    void addBit(bool bit) {
      if (bitInLastByte == 0) {
        data.push_back(0);
      }
      data.back() = data.back() | uint8_t(bit << (7-bitInLastByte));
      bitInLastByte = (bitInLastByte+1)%8;
    }
    
    void addBits(const std::vector<bool>& bits) {
      for (const bool b : bits) addBit(b);
    }
    
    void addChar(char c) {
      if (bitInLastByte == 0) {
        data.push_back(c);
      } else {
        const uint8_t msb = uint8_t(c) >> bitInLastByte;
        const uint8_t lsb = uint8_t(uint8_t(c) << (8-bitInLastByte));
        data.back() = data.back() | msb;
        data.push_back(char(lsb));
      }
    }
    
    template <typename T>
    void add(const T& data) {
      char* p = (char*)&data;
      for (size_t i = 0;i<sizeof(T);++i) {
        addChar(p[i]);
      }
    }
    
  private:
    uint8_t bitInLastByte{0};
  };

  class InputBitBuffer : public BitBuffer {
  public:
    InputBitBuffer(const std::vector<char>& data) :
    BitBuffer{data}
    {
    }

    bool getBit() {
      const bool result = data[currentByte] & (1 << (7-bitInCurrentByte));
      if (bitInCurrentByte == 7) {
        bitInCurrentByte = 0;
        ++currentByte;
      } else {
        ++bitInCurrentByte;
      }
      return result;
    }
    
    char getChar() {
      if (bitInCurrentByte == 0) {
        return char(data[currentByte++]);
      } else {
        const uint8_t msb = uint8_t(data[currentByte++] << bitInCurrentByte);
        const uint8_t lsb = uint8_t(data[currentByte]) >> (8-bitInCurrentByte);
        return char(msb | lsb);
      }
    }
    
    template <typename T>
    T get() {
      T t;
      char* p = (char*)&t;
      for (size_t i = 0;i<sizeof(T);++i) {
        p[i] = getChar();
      }
      return t;
    }
    
    bool isValid() {
      return currentByte < data.size();
    }

  private:
    uint8_t bitInCurrentByte{0};
    uint64_t currentByte{0};
  };

  void huffmanEncode(const std::string& sourceFilename, const std::string& targetFilename);
  void huffmanDecode(const std::string& sourceFilename, const std::string& targetFilename);
  std::vector<char> huffmanEncode(const std::vector<char>& source);
  std::vector<char> huffmanDecode(const std::vector<char>& source);
};
