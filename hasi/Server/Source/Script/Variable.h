#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>
#include "Primary.h"

class Variable : public Primary {
public:
  enum SpecialType {
    special_none,
    special_on,
    special_off,
    special_change
  };
  
  enum VariableType {
    system,
    input,
    state,
    pulse,
    timer,
    stopWatch,
    clock,
    random
  };
  
  Variable();
  Variable(const Variable& other);
  Variable(const Variable* other);
  Variable(const std::string& token);
  virtual ~Variable() {}

  virtual ExpressionPtr evaluate(const VarAssignments& va) const;
  virtual std::string toString() const;
  virtual std::string typeToString() const;
  virtual Variables getVariables(bool bOnlyTriggers) const;
  
  const std::string& getName() const {return varName;}
  const std::string& getRAWName() const {return rawName;}
  const VariableType& getType() const {return type;}
  const SpecialType& getSpecial() const {return special;}
  std::string getBasicName() const;
  
  bool operator<( const Variable& rhs) const;
  bool operator==( const Variable& rhs) const;
  bool operator!=( const Variable& rhs) const;
  
private:
  std::string rawName;
  std::string varName;
  VariableType type;
  SpecialType special;
  
  void detectType(const std::string& token);

};

typedef std::shared_ptr<Variable> VariablePtr;

#endif // VARIABLE_H
