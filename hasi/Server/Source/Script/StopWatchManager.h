#ifndef STOPWATCHMANAGER_H
#define STOPWATCHMANAGER_H

#include <string>
#include <vector>
#include <memory>

#ifdef _MSC_VER
  #define NOMINMAX
  #include <windows.h>
#else
  #include <sys/time.h>
#endif

class StopWatch {
public:
  StopWatch(const std::string& _name="");

  std::string save() const;
  void load(const std::string& data);

  const std::string& getName() const;
  uint32_t get() const;
  
  void reset();
  void start();
  void stop();

  std::string toString() const;
  
private:
  std::string name;
  struct timeval startVal;
  bool active;
  uint32_t accTime;
  
};

typedef std::shared_ptr<StopWatch> StopWatchPtr;

class Command;

class StopWatchManager {
public:
  StopWatchManager();
  
  bool execute(Command* cmd);
  
  void save(const std::string& filename) const;
  void load(const std::string& filename);

  uint32_t getStopWatch(const std::string& name);
  std::string toString() const;
  
private:
  std::vector<StopWatchPtr> stopWatchs;
  
  StopWatchPtr findStopWatch(const std::string& name);

  void save(const std::ofstream& file) const;
  void load(const std::ifstream& file);

  void resetStopWatch(const std::string& name);
  void startStopWatch(const std::string& name);
  void stopStopWatch(const std::string& name);
  
};

#endif // STOPWATCHMANAGER_H
