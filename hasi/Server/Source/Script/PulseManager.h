#ifndef PULSEMANAGER_H
#define PULSEMANAGER_H

#include <string>
#include <vector>
#include <memory>

class Pulse {
public:
  Pulse(const std::string& _name="");

  const std::string& getName() const;
  double get();
  void set(double v);
  
  std::string toString() const;
  
private:
  std::string name;
  double value;
  
};

typedef std::shared_ptr<Pulse> PulsePtr;

class Command;

class PulseManager {
public:
  PulseManager();
  
  bool execute(Command* cmd);
  
  double getPulse(const std::string& name);
  
  std::string toString() const;
  
private:
  std::vector<PulsePtr> Pulses;
  
  PulsePtr findPulse(const std::string& name);
  void setPulse(const std::string& name, double v);

};

#endif // PULSEMANAGER_H
