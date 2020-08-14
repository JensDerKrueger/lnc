#include "OutputCommand.h"
#include "Value.h"
#include "ExpressionExceptions.h"

#include <sstream>

std::string OutputCommand::toString() const {
  return std::string("[") + name + " (output)] = " + expression->toString();
}

OutputCommand::OutputCommand(const std::string& _name, ExpressionPtr _expression) :
ExpressionCommand(_name, _expression)
{
}

CommandPtr OutputCommand::evaluate(const VarAssignments& va) const {
  ExpressionPtr ev = expression->evaluate(va);
  
  Value* v = dynamic_cast<Value*>(ev.get());
  
  if (v) {
    CommandPtr p(new OutputCommand(name, ev));
    return p;
  } else {
    std::stringstream ss;
    ss << "Incomplete evaluation in OutputCommand execution. Command:" << toString()
    << " Evaluated expression: " << ev->toString();
    throw ExecuteException(ss.str());
  }
}

Outputs OutputCommand::getOutputs() const {
  Outputs o;
  o.insert(name);
  return o;
}
