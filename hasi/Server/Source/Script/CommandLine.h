#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <string>
#include <vector>
#include "Expression.h"
#include "Command.h"

typedef std::vector<CommandPtr> CommandVec;

class CommandLine {
public:
  CommandLine(ExpressionPtr ex, const CommandVec& cv);
  
  const Variables& getTriggers() const {return tv;}
  Variables getVariables() const;
  Outputs getOutputs() const;
  ExpressionPtr getExpression() const {return ex;}
  const CommandVec& getCommandVec() const {return cv;}
  
  std::string toString() const;
  
private:
  ExpressionPtr ex;
  CommandVec cv;
  Variables tv;
};

typedef std::shared_ptr<CommandLine> CommandLinePtr;

#endif // COMMANDLINE_H
