#pragma once

#include "Command.h"

class StopWatchCommand : public Command {
public:
  
  enum StopWatchCommandType{
    reset,
    start,
    stop
  };
  
  StopWatchCommand(const std::string& token);
  virtual ~StopWatchCommand() {}
  
  virtual CommandPtr evaluate(const VarAssignments& va) const;

  virtual std::string toString() const;
  virtual Variables getVariables() const;
  
  const StopWatchCommandType& getCmdType() const {return cmdType;}
  const std::string& getStopWatchName() const {return name;}
  
private:
  StopWatchCommand(const StopWatchCommandType& cmdType, const std::string& name);

  StopWatchCommandType cmdType;
  std::string name;
  
};
