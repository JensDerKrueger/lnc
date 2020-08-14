#include "StopWatchCommand.h"
#include "Variable.h"
#include "ExpressionExceptions.h"
#include "ParserTools.h"

StopWatchCommand::StopWatchCommand(const std::string& token)
{
  std::string tail;
  if (!ParserTools::startsWith(token, "[stopWatch_", tail)) {
    throw ParseException(token + " is not a stopWatch command");
  }
  
  if (ParserTools::startsWith(tail, "reset_", tail)) {
    cmdType = reset;
  } else
  if (ParserTools::startsWith(tail, "start_", tail)) {
    cmdType = start;
  } else
  if (ParserTools::startsWith(tail, "stop_", tail)) {
    cmdType = stop;
  } else {
    throw ParseException(token + " is a stopWatch command of an unknown type");
  }
  
  if (tail.length() < 2)
    throw ParseException(token + " has an empty stopWatch name");
  
  name = tail.substr(0, tail.length()-1);
}

StopWatchCommand::StopWatchCommand(const StopWatchCommand::StopWatchCommandType& _cmdType,
                           const std::string& _name) :
cmdType(_cmdType),
name(_name)
{
}


std::string StopWatchCommand::toString() const {
  std::string s = std::string("[") + name + std::string(" (");
  
  switch (cmdType) {
    case reset : s += "reset"; break;
    case start : s += "start"; break;
    case stop : s += "stop"; break;
  }
  
  s += ")] ";
  return s;
}

Variables StopWatchCommand::getVariables() const {
  Variables v;
  return v;  
}

CommandPtr StopWatchCommand::evaluate(const VarAssignments& ) const {
  CommandPtr p(new StopWatchCommand(cmdType,name));
  return p;
}