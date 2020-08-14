#pragma once

#include "ExpressionCommand.h"

class StateCommand : public ExpressionCommand {
public:
  StateCommand(const std::string& token);
  virtual ~StateCommand() {}
  virtual std::string toString() const;
  virtual CommandPtr evaluate(const VarAssignments& va) const;

private:
  StateCommand(const std::string& name, ExpressionPtr expression);
  
};
