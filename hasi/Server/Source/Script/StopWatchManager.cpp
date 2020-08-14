#include "StopWatchManager.h"
#include "ExpressionExceptions.h"
#include "ParserTools.h"
#include "StopWatchCommand.h"

#include <sstream>
#include <fstream>

#ifdef _MSC_VER
static int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#endif

StopWatchManager::StopWatchManager() {
}

bool StopWatchManager::execute(Command* cmd)
{
  // apply stopWatch changes
  StopWatchCommand* tCmd = dynamic_cast<StopWatchCommand*>(cmd);
  if (tCmd) {
    switch (tCmd->getCmdType()) {
      case StopWatchCommand::reset:
        resetStopWatch(tCmd->getStopWatchName());
        break;
      case StopWatchCommand::start:
        startStopWatch(tCmd->getStopWatchName());
        break;
      case StopWatchCommand::stop:
        stopStopWatch(tCmd->getStopWatchName());
        break;
    }
    return true;
  }
  return false;
}
  
void StopWatchManager::save(const std::string& filename) const {
  std::ofstream file(filename);

  for (std::vector<StopWatchPtr>::const_iterator i = stopWatchs.begin();
       i < stopWatchs.end();
       ++i) {
    file << (*i)->save() << std::endl;
  }

}

void StopWatchManager::load(const std::string& filename) {
  stopWatchs.clear();
  
  std::ifstream file(filename);
  std::string line;
  while (std::getline(file, line))
  {
    StopWatchPtr t(new StopWatch());
    t->load(line);
    stopWatchs.push_back(t);
  }
}

uint32_t StopWatchManager::getStopWatch(const std::string& name) {
  return findStopWatch(name)->get();
}

void StopWatchManager::resetStopWatch(const std::string& name) {
  return findStopWatch(name)->reset();
}

void StopWatchManager::startStopWatch(const std::string& name) {
  return findStopWatch(name)->start();
}

void StopWatchManager::stopStopWatch(const std::string& name) {
  return findStopWatch(name)->stop();
}

std::string StopWatchManager::toString() const {
  std::stringstream ss;
  for (std::vector<StopWatchPtr>::const_iterator i = stopWatchs.begin();
       i < stopWatchs.end();
       ++i) {    
    ss << (*i)->toString() << std::endl;
  }
  
  return ss.str();
}

StopWatchPtr StopWatchManager::findStopWatch(const std::string& name) {
  for (std::vector<StopWatchPtr>::const_iterator i = stopWatchs.begin();
       i < stopWatchs.end();
       ++i) {
    if ((*i)->getName() == name) {
      return *i;
    }
  }
  
  StopWatchPtr t(new StopWatch(name));
  stopWatchs.push_back(t);
  return t;
}


StopWatch::StopWatch(const std::string& _name) :
name(_name),
active(false),
accTime(0)
{
}

const std::string& StopWatch::getName() const{
  return name;
}

uint32_t StopWatch::get() const {
  if (!active) return accTime;
  
  struct timeval end;
  long mtime, seconds, useconds;
  
  gettimeofday(&end, NULL);
  
  seconds  = end.tv_sec  - startVal.tv_sec;
  useconds = end.tv_usec - startVal.tv_usec;
  
  mtime = long(((seconds) * 1000 + useconds/1000.0) + 0.5);
  
  return mtime+accTime;
}

void StopWatch::reset() {
  active = false;
  accTime = 0;
}

void StopWatch::start() {
  gettimeofday(&startVal, NULL);
  active = true;
}

void StopWatch::stop() {
  accTime = get();
  active = false;
}

std::string StopWatch::toString() const {
  std::stringstream ss;
  ss << name << " elapsed:" << get() << (active ? " (active)" : " (inactive)");
  return ss.str();
}

std::string StopWatch::save() const {
  std::stringstream ss;
  ss << name << " " << get() << " " << int(active);
  return ss.str();
}

void StopWatch::load(const std::string& data) {
  reset();
  
  std::vector<std::string> token;
  
  ParserTools::tokenize(data, token, " ");
  
  if (token.size() != 3)
    throw ParseException(std::string("Can't load stopWatch from line ") + data);
  
  name = token[0];
  accTime = atoi(token[1].c_str());
  active = atoi(token[2].c_str()) != 0;
  
  if (active) start();
}
