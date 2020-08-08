#pragma once

#include "ExpressionCommand.h"

class OutputCommand : public ExpressionCommand {
public:
  OutputCommand(const std::string& token) : ExpressionCommand(token) {}
  virtual ~OutputCommand() {}
  virtual std::string toString() const;
  virtual CommandPtr evaluate(const VarAssignments& va) const;
  virtual Outputs getOutputs() const;

private:
  OutputCommand(const std::string& name, ExpressionPtr expression);
};
