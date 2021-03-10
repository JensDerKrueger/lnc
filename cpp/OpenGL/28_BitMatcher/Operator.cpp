#include <string>

#include "Operator.h"

Operator::Operator() {}

Operator::~Operator() {}

uint32_t Operator::getOperatorCount() const {
  return 10;
}

uint8_t Operator::execute(uint8_t sourceValue, uint32_t opID, const std::string& parameter) const {
  return 0;
}
