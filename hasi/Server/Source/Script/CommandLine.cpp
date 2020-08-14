#include "CommandLine.h"
#include "Variable.h"

#include <sstream>

CommandLine::CommandLine(ExpressionPtr _ex, const CommandVec& _cv) :
ex(_ex),
cv(_cv),
tv(_ex->getVariables(true))
{
}

std::string CommandLine::toString() const {
  std::stringstream ss;
  ss << ex->toString() + " : ";
  
  for (CommandVec::const_iterator i = cv.begin();
       i != cv.end();
       i++) {
    ss << (*i)->toString() << " ";
  }

  return ss.str();
}

Variables CommandLine::getVariables() const {
  Variables v = tv;

  for (CommandVec::const_iterator i = cv.begin();
      i != cv.end();
      i++) {
    Variables vi = (*i)->getVariables();
    v.insert(vi.begin(), vi.end());
  }
  return v;
}

Outputs CommandLine::getOutputs() const {
  Outputs o;
  for (CommandVec::const_iterator i = cv.begin();
      i != cv.end();
      i++) {
    Outputs oi = (*i)->getOutputs();
    o.insert(oi.begin(), oi.end());
  }
  return o;
}
