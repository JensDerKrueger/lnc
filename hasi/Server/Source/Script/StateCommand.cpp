#include "StateCommand.h"
#include "Value.h"
#include "ExpressionExceptions.h"
#include "ParserTools.h"

#include <sstream>

StateCommand::StateCommand(const std::string& token) :
ExpressionCommand(token)
{
  std::string tail;
  if (!ParserTools::startsWith(name, "state_", tail)) {
    throw ParseException(name + " is not a state command");
  }
  name = tail;
}

StateCommand::StateCommand(const std::string& _name, ExpressionPtr _expression) :
ExpressionCommand(_name, _expression)
{
}

CommandPtr StateCommand::evaluate(const VarAssignments& va) const {
  ExpressionPtr ev = expression->evaluate(va);
  
  Value* v = dynamic_cast<Value*>(ev.get());
  
  if (v) {
    CommandPtr p(new StateCommand(name, ev));
    return p;
  } else {
    std::stringstream ss;
    ss << "Incomplete evaluation in StateCommand execution. Command:" << toString()
    << " Evaluated expression: " << ev->toString();
    throw ExecuteException(ss.str());
  }
}

std::string StateCommand::toString() const {
  return std::string("[") + name + " (state)] = " + expression->toString();
}
  
