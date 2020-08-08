#ifndef COMPOUNDEXPRESSION_H
#define COMPOUNDEXPRESSION_H

#include "Expression.h"
#include "Operator.h"

class CompoundExpression : public Expression {
public:
  CompoundExpression(ExpressionPtr lhs,
                     ExpressionPtr rhs,
                     const Operator& op);
  virtual ~CompoundExpression() {}
  
  virtual ExpressionPtr evaluate(const VarAssignments& va) const;
  virtual std::string toString() const;
  virtual Variables getVariables(bool bOnlyTriggers) const;
  
  ExpressionPtr getLhs() { return lhs; }
  ExpressionPtr getRhs() { return rhs; }
  Operator getOp() {return op;}
  
private:

  ExpressionPtr lhs;
  ExpressionPtr rhs;
  Operator op;
  
};


#endif // COMPOUNDEXPRESSION_H
