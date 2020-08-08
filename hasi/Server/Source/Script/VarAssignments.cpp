#include "VarAssignments.h"

#include <sstream>
#include "Variable.h"

VarStrAssignment::VarStrAssignment(const std::string& _name, double _value) :
name(_name),
value(_value)
{}

std::string VarStrAssignment::toString() const {
  std::stringstream ss;
  ss << name << "=" << value;
  return ss.str();
}

VarAssignment::VarAssignment(std::shared_ptr<Variable> _var, double _value) :
var(_var),
value(_value)
{}

std::string VarAssignment::toString() const {
  std::stringstream ss;
  ss << var->toString() << "= " << value;
  return ss.str();
}

