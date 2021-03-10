#pragma once

#include <string>

class Operator {
public:
  Operator();
  virtual ~Operator();
  
  uint32_t getOperatorCount() const;
  
  uint8_t execute(uint8_t sourceValue, uint32_t opID, const std::string& parameter) const;
  
};
