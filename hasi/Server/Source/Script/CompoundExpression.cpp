#include "CompoundExpression.h"
#include "Value.h"
#include "Variable.h"

CompoundExpression::CompoundExpression(const ExpressionPtr _lhs,
                                       const ExpressionPtr _rhs,
                                       const Operator& _op) :
lhs(_lhs),
rhs(_rhs),
op(_op)
{
}

ExpressionPtr CompoundExpression::evaluate(const VarAssignments& va) const {
  ExpressionPtr lhsE = lhs->evaluate(va);
  Value* lhP = dynamic_cast<Value*>(lhsE.get());

  // partial evaluation of logical and
  if (lhP && lhP->getValue() == 0.0 && op.getOpType() == Operator::logicAndOp) {
    return ExpressionPtr(new Value(0.0));
  }
  
  // partial evaluation of logical or
  if (lhP && lhP->getValue() != 0.0 && op.getOpType() == Operator::logicOrOp) {
    return ExpressionPtr(new Value(1.0));
  }
  
  ExpressionPtr rhsE = rhs->evaluate(va);
  Value* rhP = dynamic_cast<Value*>(rhsE.get());

  if (lhP && rhP) {
    return ExpressionPtr(new Value(op.execute(lhP->getValue(), rhP->getValue())));
  }
  
  return ExpressionPtr(new CompoundExpression(lhsE, rhsE, op));
}

std::string CompoundExpression::toString() const {
  return lhs->toString() + " " + op.toString() + " " + rhs->toString();
}

Variables CompoundExpression::getVariables(bool bOnlyTriggers) const {
  Variables v;

  Variables lhsV = lhs->getVariables(bOnlyTriggers);
  Variables rhsV = rhs->getVariables(bOnlyTriggers);
  
  lhsV.insert(rhsV.begin(), rhsV.end());
  
  return lhsV;
}
