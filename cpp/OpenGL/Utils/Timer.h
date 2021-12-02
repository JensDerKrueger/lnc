#pragma once

#include <chrono>


class Timer {
public:
  Timer();
  
  void start();
  double stop();
  
private:
  typedef std::chrono::high_resolution_clock Clock;
  Clock::time_point last;
  
  
};
