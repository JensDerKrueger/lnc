#include "TimerCommand.h"
#include "Variable.h"
#include "Value.h"
#include "ExpressionExceptions.h"
#include "ParserTools.h"
#include "ExpressionParser.h"

TimerCommand::TimerCommand(const std::string& token)
{
  std::string tail;
  if (!ParserTools::startsWith(token, "[timer_", tail)) {
    throw ParseException(token + " is not a timer command");
  }
  if (ParserTools::startsWith(tail, "stop_", tail)) {
    cmdType = stop;

    if (tail.length() < 2)
      throw ParseException(token + " has an empty timer name");
    
    name = tail.substr(0, tail.length()-1);
    expression = nullptr;
  } else {
    if (ParserTools::startsWith(tail, "set_", tail)) {
      cmdType = set;

      // this is a set command so now parse the line like an ExpressionCommand


      // split rhs and lhs
      std::string::size_type splitPos = token.find_first_of("=", 0);

      std::string lhs = token.substr(0,splitPos);
      std::string rhs = token.substr(splitPos+1);

      if (lhs.length() < 3 || lhs[0] != '[' || lhs[lhs.length()-1] != ']') {
        throw ParseException(std::string("Invalid lhs in command: ") + token);
      }

      name = lhs.substr(std::string("[timer_set_").length(),lhs.length()-(std::string("[timer_set_").length()+1));
      expression = ExpressionParser::parseArithmeticExpression(rhs);
    } else {
      if (ParserTools::startsWith(tail, "setMin_", tail)) {
        cmdType = setMin;

        // this is a setMin command so now parse the line like an ExpressionCommand


        // split rhs and lhs
        std::string::size_type splitPos = token.find_first_of("=", 0);

        std::string lhs = token.substr(0,splitPos);
        std::string rhs = token.substr(splitPos+1);

        if (lhs.length() < 3 || lhs[0] != '[' || lhs[lhs.length()-1] != ']') {
          throw ParseException(std::string("Invalid lhs in command: ") + token);
        }

        name = lhs.substr(std::string("[timer_setMin_").length(),lhs.length()-(std::string("[timer_setMin_").length()+1));
        expression = ExpressionParser::parseArithmeticExpression(rhs);
      } else {
        if (ParserTools::startsWith(tail, "setMax_", tail)) {
          cmdType = setMax;

          // this is a setMax command so now parse the line like an ExpressionCommand


          // split rhs and lhs
          std::string::size_type splitPos = token.find_first_of("=", 0);

          std::string lhs = token.substr(0,splitPos);
          std::string rhs = token.substr(splitPos+1);

          if (lhs.length() < 3 || lhs[0] != '[' || lhs[lhs.length()-1] != ']') {
            throw ParseException(std::string("Invalid lhs in command: ") + token);
          }

          name = lhs.substr(std::string("[timer_setMax_").length(),lhs.length()-(std::string("[timer_setMax_").length()+1));
          expression = ExpressionParser::parseArithmeticExpression(rhs);
        } else
          throw ParseException(token + " is a timer command of an unknown type");
      }
    }
  }
  
  setTimerValue = -1;
}

TimerCommand::TimerCommand(const TimerCommand::TimerCommandType& _cmdType,
                           const std::string& _name, float _setTimerValue) :
cmdType(_cmdType),
name(_name),
expression(nullptr),
setTimerValue(_setTimerValue)
{
}


std::string TimerCommand::toString() const {
  std::stringstream ss;
  ss << "[" << name << " (";
  
  switch (cmdType) {
    case set : {
      if (expression)
        ss << "set to " << expression->toString();
      else
        ss << "set to " << setTimerValue << " (evaluated)";
      break;
    }
    case setMax : {
      if (expression)
        ss << "setMax to " << expression->toString();
      else
        ss << "setMax to " << setTimerValue << " (evaluated)";
      break;
    }
    case setMin : {
      if (expression)
        ss << "setMin to " << expression->toString();
      else
        ss << "setMin to " << setTimerValue << " (evaluated)";
      break;
    }
    case stop : ss << "stop"; break;
  }
  
  ss << ")] ";
  return ss.str();
}

Variables TimerCommand::getVariables() const {
  switch (cmdType) {
    case set : {
     return expression->getVariables(false);
    }
    case setMax : {
      return expression->getVariables(false);
    }
    case setMin : {
      return expression->getVariables(false);
    }
    default : {
      Variables v;
      return v;
    }
  }
}

CommandPtr TimerCommand::evaluate(const VarAssignments& va) const {
  float stv = -1;
  if (cmdType == set && expression) {
    ExpressionPtr ev = expression->evaluate(va);
    Value* v = dynamic_cast<Value*>(ev.get());
    
    if (!v) {
      std::stringstream ss;
      ss << "Incomplete evaluation in timer reset execution. Command:" << toString()
      << " Evaluated expression: " << ev->toString();
      throw ExecuteException(ss.str());
    }
    
    stv = float(v->getValue());
  }
  
  CommandPtr p(new TimerCommand(cmdType, name, stv));
  return p; 
}
