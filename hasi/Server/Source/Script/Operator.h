#ifndef OPERATOR_H
#define OPERATOR_H

#include <string>
#include "ExpressionExceptions.h"

class Operator {
public:
  enum tOpType{
    eqOp,
    notEqOp,
    gtOp,
    lessOp,
    lessEqOp,
    gtEqOp,
    logicOrOp,
    logicAndOp,
    addOp,
    subOp,
    mulOp,
    modOp,
    divOp,
    andOp,
    orOp,
    empty
  };

  Operator();
  Operator(const std::string& token);

  std::string toString() const;
  double execute(double a, double b) const ;
  int getPrecedence() const;
  tOpType getOpType() const;

  static bool isValidOperator(const std::string& token);

private:
  tOpType opType;
  int precedence;
};


#endif // OPERATOR_H