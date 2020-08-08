#include "Variable.h"
#include "Value.h"
#include "ParserTools.h"

Variable::Variable() :
  rawName(""),
  varName(""),
  type(input),
  special(special_none)
{
}

Variable::Variable(const Variable* other) :
rawName(other->rawName),
varName(other->varName),
type(other->type),
special(other->special)
{
}

Variable::Variable(const Variable& other) :
rawName(other.rawName),
varName(other.varName),
type(other.type),
special(other.special)
{
}


Variable::Variable(const std::string& token)
{
  detectType(token);
}

void Variable::detectType(const std::string& token) {
  rawName = token.substr(1,token.length()-2);
  
  std::string name;
  std::string tail;
  if (ParserTools::startsWith(rawName, "change_", tail)) {
    special = special_change;
    name = tail;
  } else {
    if (ParserTools::startsWith(rawName, "on_", tail)) {
      special = special_on;
      name = tail;
    } else {
      if (ParserTools::startsWith(rawName, "off_", tail)) {
        special = special_off;
        name = tail;
      } else {
        special = special_none;
        name = rawName;
      }
    }
  }

  
  if (ParserTools::startsWith(name, "sys_", tail)) {
    varName = tail;
    type = system;
  } else {
    if (ParserTools::startsWith(name, "state_", tail)) {
      varName = tail;
      type = state;
    } else {
      if (ParserTools::startsWith(name, "pulse_", tail)) {
        varName = tail;
        type = pulse;
      } else {
        if (ParserTools::startsWith(name, "timer_", tail)) {
          varName = tail;
          type = timer;
        } else {
          if (ParserTools::startsWith(name, "stopWatch_", tail)) {
            varName = tail;
            type = stopWatch;
          } else {
            if (ParserTools::startsWith(name, "clock_", tail)) {
              varName = tail;
              type = clock;
            } else {
              if (ParserTools::startsWith(name, "random_", tail)) {
                varName = tail;
                type = random;
              } else {
                varName = name;
                type = input;
              }
            }
          }
        }
      }
    }
  }
}


ExpressionPtr Variable::evaluate(const VarAssignments& va) const {
  for (VarAssignments::const_iterator elem = va.begin(); elem < va.end(); elem++) {
    if ((*this) == *(elem->var))
      return ExpressionPtr(new Value(elem->value));
  }
  return ExpressionPtr(new Variable(this));
}

std::string Variable::getBasicName() const {
  switch (type) {
    case state     : return std::string("state_")+ varName;
    case pulse     : return std::string("pulse_")+ varName;
    case timer     : return std::string("timer_")+ varName;
    case stopWatch : return std::string("stopWatch_")+ varName;
    case clock     : return std::string("clock_")+ varName;
    case system    : return std::string("sys_")+ varName;
    case random    : return std::string("random_")+ varName;
    default        : return varName;
  }
}

std::string Variable::typeToString() const {
  switch (type) {
    case input : return "input"; break;
    case state : return "state"; break;
    case pulse : return "pulse"; break;
    case timer : return "timer"; break;
    case stopWatch : return "stopWatch"; break;
    case clock : return "clock"; break;
    case system : return "system"; break;
    case random : return "random"; break;
    default : return "";
  }
}

std::string Variable::toString() const {
  std::string s = std::string("[") + varName + std::string(" (");
  
  switch (special) {
    case special_on : s += "on_"; break;
    case special_off : s += "off_"; break;
    case special_change : s += "change_"; break;
    case special_none : break; // silence warning
  }
  
  s += typeToString();
  
  s += ")] ";
  return s;
}

Variables Variable::getVariables(bool bOnlyTriggers) const {
  Variables v;
  if (!bOnlyTriggers || type != random) {
    v.insert(*this);
  }
  return v;
}

bool Variable::operator<( const Variable& rhs ) const {
  return toString() < rhs.toString();
}

bool Variable::operator==( const Variable& rhs) const {
  return rhs.type == type && rhs.varName == varName && rhs.special == special;
}

bool Variable::operator!=( const Variable& rhs) const {
  return rhs.type != type || rhs.varName != varName || rhs.special != special;
}
