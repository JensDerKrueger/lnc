#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <chrono>

class Timer {
public:
  Timer(const std::string& _name="");

  std::string save() const;
  void load(const std::string& data);

  const std::string& getName() const;
  uint32_t get();
  
  void set(uint64_t value);
  void setMax(uint64_t value);
  void setMin(uint64_t value);
  void stop();
  bool isActive();

  std::string toString() const;
  uint32_t getLeftover();
  
private:
  std::string name;
  std::chrono::time_point<std::chrono::system_clock> startVal;
  bool active;
  uint64_t timerValue;

  int64_t computeLeftover() const;
};

typedef std::shared_ptr<Timer> TimerPtr;

class Command;

class TimerManager {
public:
  TimerManager();
  
  bool execute(Command* cmd);
  
  void save(const std::string& filename) const;
  void load(const std::string& filename);

  uint32_t getTimer(const std::string& name);
  std::string toString() const;
  
private:
  std::vector<TimerPtr> timers;
  
  TimerPtr findTimer(const std::string& name);

  void save(const std::ofstream& file) const;
  void load(const std::ifstream& file);

  void setTimer(const std::string& name, uint64_t value);
  void setMaxTimer(const std::string& name, uint64_t value);
  void setMinTimer(const std::string& name, uint64_t value);
  void stopTimer(const std::string& name);
  
};

#endif // TIMERMANAGER_H
