#include <iostream>
#include <string>

const std::string digits{"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"};

std::string reverse(const std::string& inStr) {
  std::string out{""};
  for (uint32_t index = 0;index<inStr.size();++index) {
    out = inStr[index] + out;
  }
  return out;
}

uint8_t find(char c, const std::string& str) {
  for (uint32_t index = 0;index<str.size();++index) {
    if (c == str[index]) return index;
  }
  return 255;
}

uint64_t strToInt(const std::string& inStr, uint8_t inBase) {
  uint64_t number{0};
  uint64_t multi{1};
  const std::string revInStr{reverse(inStr)};
  for (uint32_t index = 0;index<inStr.size();++index) {
    uint8_t digit = find(revInStr[index], digits);
    number += digit * multi;
    multi *= inBase;
  }
  return number;
}

std::string intToStr(uint64_t number, uint8_t outBase) {
  std::string revOutStr{};
  do {
    uint8_t digit = number % outBase;
    revOutStr += digits[digit];
    number = number / outBase;
  } while (number > 0);
  return reverse(revOutStr);
}

std::string convert(const std::string& inStr, uint8_t inBase, uint8_t outBase) {
  return intToStr(strToInt(inStr, inBase), outBase);
}

int main(int argc, char** argv) {
  std::cout << argv[1] << " -> " << convert(argv[1], strToInt(argv[2],10), strToInt(argv[3], 10)) << std::endl;
  return EXIT_SUCCESS;
}
