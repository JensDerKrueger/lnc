#pragma once

#include "Command.h"

class ScriptExecuteCommand : public Command {
public:  
  ScriptExecuteCommand(const std::string& token);
  virtual ~ScriptExecuteCommand() {}
  
  virtual CommandPtr evaluate(const VarAssignments& va) const;
  virtual std::string toString() const;
  virtual Variables getVariables() const;
  
  const std::string& getFilename() const {
    return filename;
  }
  
private:
  ScriptExecuteCommand();
  std::string filename;
  
};
