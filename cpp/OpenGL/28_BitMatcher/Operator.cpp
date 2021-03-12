#include <string>
#include <iostream>
#include <sstream>

#include "Operator.h"

Operator::Operator() :
convCount{3},
operationCount{7}
{
}

Operator::~Operator() {
}

std::string Operator::genOpText(uint32_t opID) {
  std::pair<uint32_t, uint32_t> opIDs = splitID(opID);
  
  std::string operant;
  switch (opIDs.first) {
    case 0: operant = "length mod 8"; break;
    case 1: operant = "first ascii"; break;
    case 2: operant = "first digit mod 8"; break;
  }
  
  std::stringstream ss;
  std::string result;
  switch (opIDs.second) {
    case 0:
      result = "NOT current";
      break;
    case 1:
      ss << "rotl( " << operant << " )";
      result = ss.str();
      break;
    case 2:
      ss << "rotr( " << operant << " )";
      result = ss.str();
      break;
    case 3:
      ss << "toggleBit( " << operant << " )";
      result = ss.str();
      break;
    case 4:
      ss << "curent XOR (" << operant << ")";
      result = ss.str();
      break;
    case 5:
      ss << "curent AND (" << operant << ")";
      result = ss.str();
      break;
    case 6:
      ss << "curent OR (" << operant << ")";
      result = ss.str();
      break;
  }
  
  return result;
}

uint8_t Operator::execute(uint8_t sourceValue, uint32_t opID, const std::string& parameter) const {
  std::pair<uint32_t, uint32_t> opIDs = splitID(opID);
  return applyOperation(sourceValue, opIDs.second, stringToNumber(opIDs.first, parameter));
}

uint32_t Operator::getOperatorCount() const {
  return convCount * operationCount;
}

std::pair<uint32_t, uint32_t> Operator::splitID(uint32_t combinedOpID) const {
  return std::make_pair(combinedOpID%convCount, std::min(combinedOpID/convCount, operationCount-1));
}

uint32_t Operator::stringToNumber(uint32_t opID, const std::string& parameter) const {
  switch (opID) {
    case 0 : return uint32_t(parameter.length());
    case 1 : return (parameter.empty()) ? 0 : uint32_t(parameter[0]);
    case 2 :
      try {
        return uint32_t(std::stoi(parameter));
      } catch (const std::invalid_argument&) {
        return 0;
      } catch (const std::out_of_range&) {
        return 0;
      }
    default : return 0;
  }
}

uint8_t Operator::rotl(uint8_t v, uint32_t shift) const {
  uint8_t s =  uint8_t(shift>=0? shift%8 : -((-shift)%8));
  return uint8_t((v<<s) | (v>>(8-s)));
}

uint8_t Operator::rotr(uint8_t v, uint32_t shift) const {
  uint8_t s =  uint8_t(shift>=0? shift%8 : -((-shift)%8));
  return uint8_t((v>>s) | (v<<(8-s)));
}

uint8_t Operator::toggle(uint8_t v, uint32_t bit) const {
  return uint8_t(v ^ rotl(1, bit));
}

uint8_t Operator::applyOperation(uint8_t sourceValue, uint32_t opID, uint32_t parameter) const {
  switch (opID) {
    case 0 : return ~sourceValue;
    case 1 : return rotl(sourceValue, parameter);
    case 2 : return rotr(sourceValue, parameter);
    case 3 : return toggle(sourceValue, parameter);
    case 4 : return sourceValue ^ uint8_t(parameter);
    case 5 : return sourceValue & uint8_t(parameter);
    case 6 : return sourceValue | uint8_t(parameter);
    default : return 0;
  }
}
