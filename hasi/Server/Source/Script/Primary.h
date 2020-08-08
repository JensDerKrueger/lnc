#ifndef PRIMARY_H
#define PRIMARY_H

#include "Expression.h"
#include <string>

class Primary;
typedef std::shared_ptr<Primary> PrimaryPtr;

class Primary : public Expression {
public:
  static PrimaryPtr StringToPrimary(const std::string& token);
};

#endif // PRIMARY_H