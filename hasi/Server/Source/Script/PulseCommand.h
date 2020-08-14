#pragma once

#include "ExpressionCommand.h"

class PulseCommand : public ExpressionCommand {
public:
  PulseCommand(const std::string& token);
  virtual ~PulseCommand() {}
  virtual std::string toString() const;
  virtual CommandPtr evaluate(const VarAssignments& va) const;

private:
  PulseCommand(const std::string& name, ExpressionPtr expression);
  
};
