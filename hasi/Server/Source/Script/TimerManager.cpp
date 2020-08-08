#include "TimerManager.h"
#include "ExpressionExceptions.h"
#include "ParserTools.h"
#include "TimerCommand.h"

#include <sstream>
#include <fstream>

TimerManager::TimerManager() {
}

bool TimerManager::execute(Command* cmd)
{
  // apply timer changes
  TimerCommand* tCmd = dynamic_cast<TimerCommand*>(cmd);
  if (tCmd) {
    switch (tCmd->getCmdType()) {
      case TimerCommand::set:
        setTimer(tCmd->getTimerName(), uint32_t(tCmd->getSetTimerValue()+0.5));
        break;
      case TimerCommand::setMax:
        setMaxTimer(tCmd->getTimerName(), uint32_t(tCmd->getSetTimerValue()+0.5));
        break;
      case TimerCommand::setMin:
        setMinTimer(tCmd->getTimerName(), uint32_t(tCmd->getSetTimerValue()+0.5));
        break;
      case TimerCommand::stop:
        stopTimer(tCmd->getTimerName());
        break;
    }
    return true;
  }
  return false;
}
  
void TimerManager::save(const std::string& filename) const {
  std::ofstream file(filename);

  for (std::vector<TimerPtr>::const_iterator i = timers.begin();
       i < timers.end();
       ++i) {
    file << (*i)->save() << std::endl;
  }

}

void TimerManager::load(const std::string& filename) {
  timers.clear();
  
  std::ifstream file(filename);
  std::string line;
  while (std::getline(file, line))
  {
    TimerPtr t(new Timer());
    t->load(line);
    timers.push_back(t);
  }
}

uint32_t TimerManager::getTimer(const std::string& name) {
  std::string tail;
  if (ParserTools::startsWith(name, "remaining_", tail)) {
    return findTimer(tail)->getLeftover();
  } else {
    if (ParserTools::startsWith(name, "active_", tail)) {
      return findTimer(tail)->isActive() ? 1 : 0;
    } else {
      return findTimer(name)->get();
    }
  }
}

void TimerManager::setTimer(const std::string& name, uint64_t value) {
  findTimer(name)->set(value);
}

void TimerManager::setMaxTimer(const std::string& name, uint64_t value) {
  findTimer(name)->setMax(value);
}

void TimerManager::setMinTimer(const std::string& name, uint64_t value) {
  findTimer(name)->setMin(value);
}

void TimerManager::stopTimer(const std::string& name) {
  findTimer(name)->stop();
}

std::string TimerManager::toString() const {
  std::stringstream ss;
  for (std::vector<TimerPtr>::const_iterator i = timers.begin();
       i < timers.end();
       ++i) {    
    ss << (*i)->toString() << std::endl;
  }
  
  return ss.str();
}

TimerPtr TimerManager::findTimer(const std::string& name) {
  for (std::vector<TimerPtr>::const_iterator i = timers.begin();
       i < timers.end();
       ++i) {
    if ((*i)->getName() == name) {
      return *i;
    }
  }
  
  TimerPtr t(new Timer(name));
  timers.push_back(t);
  return t;
}


Timer::Timer(const std::string& _name) :
name(_name),
active(false)
{
}

const std::string& Timer::getName() const{
  return name;
}

int64_t Timer::computeLeftover() const {
  auto current=std::chrono::system_clock::now();
  auto waitedFor=current-startVal;
  auto elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(waitedFor).count();

  return int64_t(timerValue)-int64_t(elapsedMillis);
}


uint32_t Timer::getLeftover() {
  if (!active) return 0;

  int64_t lo = computeLeftover();

  
  if (lo < 0) {

    // time expired for more than 100 seconds and has still not been
    // made inactive -> than deactive it
    // this can happen when the user never calls a get()
    // on the time but only getLeftover() and/or isActive()
    if (lo < -1000*100) {
      active = false;
    }

    return 0;
  } else {
    return uint32_t(lo);
  }
}


uint32_t Timer::get() {
  if (active && getLeftover() == 0) {
    active = false;
    return 1;
  } else {
    return 0;
  }
}

void Timer::stop() {
  active = false;
}

void Timer::set(uint64_t value) {
  startVal=std::chrono::system_clock::now();
  active = true;
  timerValue = value;
}

void Timer::setMax(uint64_t value) {
  uint64_t lo = getLeftover();
  if (lo < value) set(value);
}

void Timer::setMin(uint64_t value) {
  uint64_t lo = computeLeftover();
  if (lo > value) set(value);
}

bool Timer::isActive() {
  return active && getLeftover() > 0;
}

std::string Timer::toString() const {
  std::stringstream ss;
  ss << name << " remaining runtime: " << computeLeftover() << (active ? " (active)" : " (inactive)");
  return ss.str();
}

std::string Timer::save() const {
  std::stringstream ss;

  int64_t lo = computeLeftover();

  if (lo < 0)
    ss << name << " 0 0";
  else
    ss << name << " " << lo << " " << int(active);

  return ss.str();
}

void Timer::load(const std::string& data) {
  std::vector<std::string> token;
  
  ParserTools::tokenize(data, token, " ");
  
  if (token.size() != 3)
    throw ParseException(std::string("Can't load timer from line ") + data);
  
  name = token[0];
  timerValue = atoi(token[1].c_str());
  active = atoi(token[2].c_str()) != 0;
  
  if (active) set(timerValue);
}
