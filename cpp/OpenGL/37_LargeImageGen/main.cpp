#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>

#include <Vec3.h>
#include <bmp.h>

#include "Fractal.h"

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


Vec3t<uint8_t> applyTransferFunction(uint8_t input) {
  return {input, uint8_t(input*3), uint8_t(input*25)};
}

void applyTransferFunction(const std::vector<uint8_t>& inputData,
                           std::vector<uint8_t>& outputImage) {
  
  for (size_t i = 0;i<inputData.size();++i) {
    Vec3t<uint8_t> color = applyTransferFunction(inputData[i]);
    
    outputImage[i*4+0] = color.r;
    outputImage[i*4+1] = color.g;
    outputImage[i*4+2] = color.b;
    outputImage[i*4+3] = 255;
  }
}


int main(int argc, char** argv) {
  const uint32_t totalWidth{65536};
  const uint32_t totalHeight{65536};
  const uint32_t tileWidth{512};
  const uint32_t tileHeight{512};
  const std::string filename{"fractal.dat"};

  std::ofstream file(filename, std::ios::binary);
  
  Fractal f(tileWidth,tileHeight,totalWidth,totalHeight,0,0,selectOpenCLDevice());
  
  std::cout << "Starting fractal computation ... " << std::flush;
  auto t1 = Clock::now();

  std::vector<uint8_t> image(size_t(tileHeight)*size_t(tileWidth)*4);
  for (uint32_t tileY = 0; tileY < totalHeight/tileHeight; ++tileY) {
    for (uint32_t tileX = 0; tileX < totalWidth/tileWidth; ++tileX) {
      f.setOffset(tileX*tileWidth, tileY*tileHeight);
      f.compute();
      applyTransferFunction(f.getData(), image);
      /*
      std::stringstream bmpFilename;
      bmpFilename << "tile_" << tileX << "_" << tileY << ".bmp";
      BMP::save(bmpFilename.str(), tileWidth, tileHeight, image, 4);
      */
      file.write((char*)image.data(), image.size());
    }
  }
  
  auto t2 = Clock::now();
  std::cout << " done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " milliseconds!" << std::endl;
  

  file.close();

  return EXIT_SUCCESS;
}
