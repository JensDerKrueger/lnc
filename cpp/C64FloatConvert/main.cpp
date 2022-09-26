#include <iostream>
#include <array>
#include <bitset>
#include <iomanip>

int main(int argc, char ** argv) {
  double value;
  std::cout << "Float:";
  std::cin >> value;

  if (value == 0) {
    std::cout << "$00, $00, $00, $00, $00" << std::endl;
    return EXIT_SUCCESS;
  }

  bool sign = value < 0;
  if (sign) value *= -1;

  uint32_t integerPart = uint32_t(value);
  double fractionPart = value-double(integerPart);

  int32_t exponent;
  if (value >= 1) {
    exponent = -1;
    uint32_t tmp = integerPart;
    while (tmp > 0) {
      tmp /= 2;
      exponent++;
    }
  } else {
    exponent = 0;
    while (fractionPart < 1) {
      fractionPart *= double(2);
      exponent--;
    }
  }

  // integer part
  std::bitset<32> bits(integerPart);
  bits = (bits<<(31-exponent));
  
  // sign bit
  bits[31] = sign;

  // fractional part
  const size_t intBits = (exponent > 0) ? size_t(exponent) : 0;
  for (size_t i = intBits;i<32;++i) {
    bits[31-i] = bits[31-i] | int(fractionPart);
    fractionPart = (fractionPart-double(int(fractionPart))) * 2.0;
  }
  
  // convert 32-bit bitset to 4 seperate bytes
  std::array<std::bitset<8>,4> mantissa;
  size_t j = 4;
  for (size_t i = 0;i<32;++i) {
    if (i%8 == 0) j--;
    mantissa[j][i%8] = bits[i];
  }

  // output
  std::cout << "$" << std::hex << std::setfill('0') << std::setw(2) << exponent+129 << ", "
            << "$" << std::setfill('0') << std::setw(2)<< mantissa[0].to_ulong() << ", "
            << "$" << std::setfill('0') << std::setw(2)<< mantissa[1].to_ulong() << ", "
            << "$" << std::setfill('0') << std::setw(2)<< mantissa[2].to_ulong() << ", "
            << "$" << std::setfill('0') << std::setw(2)<< mantissa[3].to_ulong() << std::endl;
  
  return EXIT_SUCCESS;
}

