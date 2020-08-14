#include "ActivationCommand.h"

#include "Variable.h"
#include "ExpressionExceptions.h"
#include "ParserTools.h"

#include <sstream>


ActivationCommand::ActivationCommand(const std::string& token) {
  
  std::string tail;
  if (ParserTools::startsWith(token, "[activate_", tail)) {
    cmdType = activate;
  } else {
    if (ParserTools::startsWith(token, "[deactivate_", tail)) {
      cmdType = deactivate;      
    } else {
      throw ParseException(token + " is not an activation command");
    }
  }
  
  if (tail.length() < 2)
    throw ParseException(token + " has an empty device name");
  
  deviceName = tail.substr(0, tail.length()-1);
}

ActivationCommand::ActivationCommand(ActivationCommandType ct, const std::string& dn)
: cmdType(ct)
, deviceName(dn)
{
}

CommandPtr ActivationCommand::evaluate(const VarAssignments& va) const {
  ActivationCommand* d = new ActivationCommand(cmdType, deviceName);
  CommandPtr p(d);
  return p;
}

std::string ActivationCommand::toString() const {
  std::stringstream ss;
  if (cmdType == activate)
    ss << "[Activation command for " << deviceName << ")]";
  else
    ss << "[Deactivation command for " << deviceName << ")]";
  return ss.str();
}

Variables ActivationCommand::getVariables() const {
  Variables v;
  return v;
}

