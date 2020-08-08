#include "Primary.h"

#include "Variable.h"
#include "Value.h"

PrimaryPtr Primary::StringToPrimary(const std::string& token) {
  PrimaryPtr p;
  if (token.length() > 2 && token[0] == '[') {
    p = VariablePtr(new Variable(token));
  } else {
    p = std::shared_ptr<Value>(new Value(token));
  }
  return p;
}
