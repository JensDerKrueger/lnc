#include "Command.h"
#include "OutputCommand.h"
#include "StateCommand.h"
#include "PulseCommand.h"
#include "TimerCommand.h"
#include "StopWatchCommand.h"
#include "ScriptExecuteCommand.h"
#include "ParserTools.h"
#include "ExpressionExceptions.h"
#include "ActivationCommand.h"

CommandPtr Command::StringToCommand(const std::string& token) {
  if (ParserTools::startsWith(token, "[script_")) {
    return CommandPtr(new ScriptExecuteCommand(token));
  } else
  if (ParserTools::startsWith(token, "[timer_")) {
    return CommandPtr(new TimerCommand(token));
  } else
  if (ParserTools::startsWith(token, "[stopWatch_")) {
    return CommandPtr(new StopWatchCommand(token));
  } else
  if (ParserTools::startsWith(token, "[state_")) {
    return CommandPtr(new StateCommand(token));
  } else
  if (ParserTools::startsWith(token, "[pulse_")) {
    return CommandPtr(new PulseCommand(token));
  } else
  if (ParserTools::startsWith(token, "[activate_") ||
      ParserTools::startsWith(token, "[deactivate_" )) {
    return CommandPtr(new ActivationCommand(token));
  } else
  if (ParserTools::startsWith(token, "[sys_")) {
    throw ParseException(std::string("System values are readonly. Assignment found in \"") + token + std::string("\""));
  } else {
    return CommandPtr(new OutputCommand(token));
  }
}

Outputs Command::getOutputs() const {
  Outputs o;
  return o;
}
