#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <memory>
#include "Expression.h"

class Command;
typedef std::shared_ptr<Command> CommandPtr;

class Command {
public:
  static CommandPtr StringToCommand(const std::string& token);

  virtual CommandPtr evaluate(const VarAssignments& va) const = 0;
  virtual std::string toString() const = 0;
  virtual Variables getVariables() const = 0;
  virtual Outputs getOutputs() const;
};

#endif // COMMAND_H
