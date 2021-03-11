#pragma once

#include <string>

class Operator {
public:
  Operator();
  virtual ~Operator();
  
  uint32_t getOperatorCount() const;
  uint8_t execute(uint8_t sourceValue, uint32_t opID, const std::string& parameter) const;
  
private:
  uint32_t convCount;
  uint32_t operationCount;
  
  std::pair<uint32_t, uint32_t> splitID(uint32_t combinedOpID) const;
  uint32_t stringToNumber(uint32_t opID, const std::string& parameter) const;
  uint8_t applyOperation(uint8_t sourceValue, uint32_t opID, uint32_t parameter) const;

  uint8_t rotl(uint8_t v, uint32_t shift) const;
  uint8_t rotr(uint8_t v, uint32_t shift) const;
  uint8_t toggle(uint8_t v, uint32_t bit) const;
};
