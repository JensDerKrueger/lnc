#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <vector>
#include <set>
#include <string>

#include "VarAssignments.h"

class Expression;
typedef std::shared_ptr<Expression> ExpressionPtr;

class Expression {
public:
  virtual ExpressionPtr evaluate(const VarAssignments& va) const = 0;
  virtual std::string toString() const = 0;
  virtual Variables getVariables(bool bOnlyTriggers) const = 0;
  
};


#endif // EXPRESSION_H