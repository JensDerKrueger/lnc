#pragma once

#include "Command.h"

class ActivationCommand : public Command {
public:
  
  enum ActivationCommandType{
    activate,
    deactivate
  };  

  ActivationCommand(const std::string& token);
  virtual ~ActivationCommand() {}
  virtual CommandPtr evaluate(const VarAssignments& va) const;
  virtual std::string toString() const;
  virtual Variables getVariables() const;
  
  const std::string& getDeviceName() const {
    return deviceName;
  }

  const ActivationCommandType& getCmdType() const {
    return cmdType;
  }

  
  
private:
  ActivationCommand(ActivationCommandType ct, const std::string& dn);

  ActivationCommandType cmdType;
  std::string deviceName;
  
};
