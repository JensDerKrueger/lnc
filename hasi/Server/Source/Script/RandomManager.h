#ifndef RANDOMMANAGER_H
#define RANDOMMANAGER_H

#include <string>
#include <random>

class RandomManager {
public:
  RandomManager();
  
  double getRandom(const std::string& name);
  
  std::string toString() const;
  
private:
  std::mt19937 m_rng;
  
};

#endif // RANDOMMANAGER_H