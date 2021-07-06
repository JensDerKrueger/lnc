#include <iostream>
#include <chrono>

#include "MultiresGen.h"

#include <OpenClUtils.h>

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char** argv) {
  cl_device_id dev = selectOpenCLDevice();  
  MultiresGen multi(1<<19,512,1);
  
  auto t1 = Clock::now();
  multi.generate(dev, "/Volumes/LaCie RAID VD 0/fractal_19.dat");
  auto t2 = Clock::now();

  std::cout << "Done in "
            << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
            << " milliseconds!" << std::endl;

  return EXIT_SUCCESS;
}
