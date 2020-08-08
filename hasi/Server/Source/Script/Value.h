#ifndef VALUE_H
#define VALUE_H

#include <string>
#include "Primary.h"

class Value : public Primary {
public:
  Value();
  Value(const Value* other);
  Value(const std::string& token);
  Value(double value);
  virtual ~Value() {}

  virtual ExpressionPtr evaluate(const VarAssignments& va) const;
  virtual std::string toString() const;
  virtual Variables getVariables(bool bOnlyTriggers) const;
  
  double getValue() const {return value;}
  
private:
  double value;

};

#endif // VALUE_H
