#include "Operator.h"

#include <sstream>

Operator::Operator() {
  opType = empty;
  precedence = 0;
}

Operator::Operator(const std::string& token) {

  if (token == "&&") {opType = andOp; precedence = 0;  return;}
  if (token == "||") {opType = orOp; precedence = 0;  return;}
  if (token == "|")  {opType = logicOrOp; precedence = 1;  return;}
  if (token == "&")  {opType = logicAndOp; precedence = 1;  return;}
  if (token == "==") {opType = eqOp; precedence = 2; return;}
  if (token == "!=") {opType = notEqOp; precedence = 2; return;}
  if (token == ">=") {opType = gtEqOp; precedence = 2;  return;}
  if (token == "<=") {opType = lessEqOp; precedence = 2; return;}
  if (token == ">")  {opType = gtOp; precedence = 2;  return;}
  if (token == "<")  {opType = lessOp; precedence = 2; return;}
  if (token == "+")  {opType = addOp; precedence = 3;  return;}
  if (token == "-")  {opType = subOp; precedence = 3;  return;}
  if (token == "*")  {opType = mulOp; precedence = 4;  return;}
  if (token == "/")  {opType = divOp; precedence = 4;  return;}
  if (token == "%")  {opType = modOp; precedence = 4;  return;}
  if (token == "")   {opType = empty; precedence = 0; return;}

  throw ParseException(std::string("invalid token \"") + token + std::string("\""));
}

bool Operator::isValidOperator(const std::string& token) {
  return token == "&" || token == "|" || token == "=" || token == "!" ||
  token == "&&" || token == "||" || token == "==" || token == "!=" ||
  token == ">=" || token == "<=" || token == ">" || token == "<" ||
  token == "+" || token == "-" || token == "*" || token == "/" ||
  token == "%" || token == "";
}

std::string Operator::toString() const {
  switch (opType) {
    case eqOp : return "==";
    case notEqOp  : return "!=";
    case lessOp : return "<";
    case gtOp  : return ">";
    case lessEqOp : return "<=";
    case gtEqOp  : return ">=";
    case andOp : return "&&";
    case orOp  : return "||";
    case logicAndOp : return "&";
    case logicOrOp  : return "|";
    case addOp : return "+";
    case subOp : return "-";
    case mulOp : return "*";
    case divOp : return "/";
    case modOp : return "%";
    default : return "";
  }
}

double Operator::execute(double a, double b) const {
  switch (opType) {
    case andOp : return double(a && b);
    case orOp  : return double(a || b);
    case logicAndOp : return double(int(a) & int(b));
    case logicOrOp  : return double(int(a) | int(b));
    case eqOp : return double(a == b);
    case notEqOp  : return double(a != b);
    case gtEqOp : return double(a >= b);
    case lessEqOp  : return double(a <= b);
    case gtOp : return double(a > b);
    case lessOp  : return double(a < b);
    case addOp : return a+b;
    case subOp : return a-b;
    case mulOp : return a*b;
    case divOp : return a/b;
    case modOp : return int(a)%int(b);
    default :
      std::stringstream ss;
      ss << "trying to execute empty operator with " << a << " and " << b;
      throw ExecuteException(ss.str());
  }
}

int Operator::getPrecedence() const {
  return precedence;
}

Operator::tOpType Operator::getOpType() const {
  return opType;
}
