#ifndef CLOCKMANAGER_H
#define CLOCKMANAGER_H

#include <string>

class ClockManager {
public:
  ClockManager();
  
  double getClock(const std::string& name) const;
  
  std::string toString() const;
  
};

#endif // CLOCKMANAGER_H