#include "Timer.h"

Timer::Timer() {
  start();
}

void Timer::start() {
  last = Clock::now();
}
  
  
double Timer::stop() {
  auto now = Clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(now - last).count()/1000.0;
}
