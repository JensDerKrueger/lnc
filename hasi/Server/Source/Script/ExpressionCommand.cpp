#include "ExpressionCommand.h"
#include "Variable.h"
#include "Value.h"
#include "ExpressionExceptions.h"
#include "ParserTools.h"
#include "ExpressionParser.h"

#include <sstream>
#include <iostream>

ExpressionCommand::ExpressionCommand(const std::string& token) :
  expression(nullptr),
  name("")
{
  // split rhs and lhs
  std::string::size_type splitPos = token.find_first_of("=", 0);

  if (std::string::npos == splitPos) {
    throw ParseException(std::string("Missing assignment in command: ") + token);
  }
  
  std::string lhs = token.substr(0,splitPos);
  std::string rhs = token.substr(splitPos+1);
  

  if (lhs.length() < 3 || lhs[0] != '[' || lhs[lhs.length()-1] != ']') {
    throw ParseException(std::string("Invalid lhs in command: ") + token);
  }
  
  name = lhs.substr(1,lhs.length()-2);
  
  expression = ExpressionParser::parseArithmeticExpression(rhs);
}

ExpressionCommand::ExpressionCommand(const std::string& _name, ExpressionPtr _expression) :
  expression(_expression),
  name(_name)
{
}

std::string ExpressionCommand::toString() const {
  return std::string("[") + name + "] = " + expression->toString();
}

Variables ExpressionCommand::getVariables() const {
  return expression->getVariables(false);
}

const double ExpressionCommand::getEvaluatedExpressionValue() const {
  Value* v = dynamic_cast<Value*>(expression.get());
  
  if (!v) {
    throw ParseException(std::string("Error calling getEvaluatedExpressionValue on a non-value expression"));
  }
  
  return v->getValue();
}
