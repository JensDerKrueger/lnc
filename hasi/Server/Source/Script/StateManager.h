#ifndef STATEMANAGER_H
#define STATEMANAGER_H

#include <string>
#include <vector>
#include <memory>

class State {
public:
  State(const std::string& _name="");

  std::string save() const;
  void load(const std::string& data);

  const std::string& getName() const;
  double get() const;
  void set(double v);
  
  std::string toString() const;
  
private:
  std::string name;
  double value;
  
};

typedef std::shared_ptr<State> StatePtr;

class Command;

class StateManager {
public:
  StateManager();
  
  bool execute(Command* cmd);
  
  void save(const std::string& filename) const;
  void load(const std::string& filename);
  
  double getState(const std::string& name);
  
  std::string toString() const;
  
private:
  std::vector<StatePtr> States;
  
  StatePtr findState(const std::string& name);

  void save(const std::ofstream& file) const;
  void load(const std::ifstream& file);
  
  void setState(const std::string& name, double v);
};

#endif // STATEMANAGER_H
