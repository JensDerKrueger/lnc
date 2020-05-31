#include <iostream>
#include "Fractal.h"

#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char** argv) {
  Fractal f(4096,4096);
  
  std::cout << "Starting fractal computation ... " << std::flush;
  auto t1 = Clock::now();
  f.compute();
  auto t2 = Clock::now();
  std::cout << " done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " milliseconds!" << std::endl;
  
  f.save("fractal.bmp");
  return EXIT_SUCCESS;
}
