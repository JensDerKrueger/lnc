#pragma once

#include "Command.h"
#include "Expression.h"

class TimerCommand : public Command {
public:
  
  enum TimerCommandType{
    set,
    setMax,
    setMin,
    stop
  };
  
  TimerCommand(const std::string& token);
  virtual ~TimerCommand() {}
  
  virtual CommandPtr evaluate(const VarAssignments& va) const;

  virtual std::string toString() const;
  virtual Variables getVariables() const;
  
  const TimerCommandType& getCmdType() const {return cmdType;}
  const std::string& getTimerName() const {return name;}
  
  float getSetTimerValue() const {return setTimerValue;}
  
private:
  TimerCommand(const TimerCommandType& cmdType, const std::string& name, float setTimerValue);

  TimerCommandType cmdType;
  std::string name;
  ExpressionPtr expression;
  float setTimerValue;
  
};
