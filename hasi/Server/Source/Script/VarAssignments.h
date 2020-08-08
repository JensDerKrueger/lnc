#ifndef VARASSIGNEMENTS_H
#define VARASSIGNEMENTS_H

#include <vector>
#include <set>
#include <string>
#include <memory>

class VarStrAssignment {
public:
  VarStrAssignment(const std::string& name, double value);
  std::string toString() const;
  
  std::string name;
  double value;
  
};
typedef std::vector<VarStrAssignment> VarStrAssignments;

class Variable;

class VarAssignment {
public:
  VarAssignment(std::shared_ptr<Variable> var, double value);
  std::string toString() const;
  
  std::shared_ptr<Variable> var;
  double value;
};

typedef std::vector<VarAssignment> VarAssignments;
typedef std::set<Variable> Variables;
typedef std::set<std::string> Outputs;


#endif // VARASSIGNEMENTS_H
