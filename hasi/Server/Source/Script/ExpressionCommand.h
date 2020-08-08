#ifndef EXPRESSIONCOMMAND_H
#define EXPRESSIONCOMMAND_H

#include "Command.h"
#include "Expression.h"

class ExpressionCommand : public Command{
public:
  ExpressionCommand(const std::string& token);
  
  virtual std::string toString() const;
  virtual Variables getVariables() const;
  const std::string& getExpressionName() const {return name;}
  const double getEvaluatedExpressionValue() const;
  
protected:
  ExpressionCommand(const std::string& name, ExpressionPtr expression);

  ExpressionPtr expression;
  std::string name;
  
};

#endif // EXPRESSIONCOMMAND_H
