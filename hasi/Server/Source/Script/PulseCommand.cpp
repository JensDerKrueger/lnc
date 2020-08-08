#include "PulseCommand.h"
#include "Value.h"
#include "ExpressionExceptions.h"
#include "ParserTools.h"

#include <sstream>

PulseCommand::PulseCommand(const std::string& token) :
ExpressionCommand(token)
{
  std::string tail;
  if (!ParserTools::startsWith(name, "pulse_", tail)) {
    throw ParseException(name + " is not a Pulse command");
  }
  name = tail;
}

PulseCommand::PulseCommand(const std::string& _name, ExpressionPtr _expression) :
ExpressionCommand(_name, _expression)
{
}

CommandPtr PulseCommand::evaluate(const VarAssignments& va) const {
  ExpressionPtr ev = expression->evaluate(va);
  
  Value* v = dynamic_cast<Value*>(ev.get());
  
  if (v) {
    CommandPtr p(new PulseCommand(name, ev));
    return p;
  } else {
    std::stringstream ss;
    ss << "Incomplete evaluation in PulseCommand execution. Command:" << toString()
    << " Evaluated expression: " << ev->toString();
    throw ExecuteException(ss.str());
  }
}

std::string PulseCommand::toString() const {
  return std::string("[") + name + " (Pulse)] = " + expression->toString();
}
  
