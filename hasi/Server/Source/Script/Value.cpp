#include "Value.h"
#include "Variable.h"
#include <sstream>
#include "ParserTools.h"

Value::Value() :
  value(0)
{
}

Value::Value(const Value* other) :
  value(other->value)
{
}

Value::Value(const std::string& token)
{
  value = ParserTools::fromString<double>(token);
}

Value::Value(double _value) :
value(_value)
{
}

ExpressionPtr Value::evaluate(const VarAssignments& ) const {
  return ExpressionPtr(new Value(this));
}

std::string Value::toString() const {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

Variables Value::getVariables(bool) const {
  Variables v;
  return v;
}

