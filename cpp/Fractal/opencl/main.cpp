#include <iostream>
#include "Fractal.h"

#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

cl_device_id selectOpenCLDevice() {
  const auto platforms = OpenClContext<unsigned char>::getInfo();
  size_t i = 0;
  for (const PlatformInfo& platform : platforms) {
    for (const DeviceInfo& d : platform.devices) {
      std::cout << i++ << " -> " << d.name << std::endl;
    }
  }
  
  // if there is only one device in the system, return that device
  if (i == 1) {
    return platforms[0].devices[0].deviceID;
  }
  
  cl_device_id selectedDevice{0};
  size_t selectedDeviceIndex{0};
  do {
    std::cout << "Select a device number: ";  std::cin >> selectedDeviceIndex;
    i = 0;
    for (const PlatformInfo& platform : platforms) {
      for (const DeviceInfo& d : platform.devices) {
        if (i == selectedDeviceIndex) {
          selectedDevice =  d.deviceID;
          std::cout << "Selected " << d.name << std::endl;
          break;
        }
        i++;
      }
      if (selectedDevice != 0) break;
    }
  } while (selectedDevice == 0);
  return selectedDevice;
}

int main(int argc, char** argv) {
  Fractal f(4096,4096, selectOpenCLDevice());
  
  std::cout << "Starting fractal computation ... " << std::flush;
  auto t1 = Clock::now();
  f.compute();
  auto t2 = Clock::now();
  std::cout << " done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " milliseconds!" << std::endl;
  
  f.save("fractal.bmp");
  return EXIT_SUCCESS;
}
