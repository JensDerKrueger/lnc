#include "ScriptExecuteCommand.h"
#include "Variable.h"
#include "ExpressionExceptions.h"
#include "ParserTools.h"

#include <sstream>

ScriptExecuteCommand::ScriptExecuteCommand(const std::string& token)
{

  std::string tail;
  if (!ParserTools::startsWith(token, "[script_", tail)) {
    throw ParseException(token + " is not an ScriptExecute command");
  }

  if (tail.length() < 2)
    throw ParseException(token + " has an empty ScriptExecute filename");

  filename = tail.substr(0, tail.length()-1);
}

ScriptExecuteCommand::ScriptExecuteCommand()
{
}

CommandPtr ScriptExecuteCommand::evaluate(const VarAssignments& ) const {
  ScriptExecuteCommand* d = new ScriptExecuteCommand();
  d->filename = filename;
  CommandPtr p(d);
  return p;
}

std::string ScriptExecuteCommand::toString() const {
  std::stringstream ss;
  ss << "[Script execution (from config file " << filename << ")]";
  return ss.str();
}

Variables ScriptExecuteCommand::getVariables() const {
  Variables v;
  return v;
}
