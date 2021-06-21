#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>

#include <Vec3.h>
#include <bmp.h>

#include "Fractal.h"

typedef std::chrono::high_resolution_clock Clock;

static cl_device_id selectOpenCLDevice() {
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
//    std::cout << "Select a device number: ";  std::cin >> selectedDeviceIndex;
    selectedDeviceIndex = 1;
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


static Vec3t<uint8_t> applyTransferFunction(uint8_t input) {
  return {input, uint8_t(input*3), uint8_t(input*25)};
}

static void applyTransferFunction(const std::vector<uint8_t>& inputData,
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
  constexpr uint32_t totalSize{2048};
  constexpr uint32_t tileSize{512};
  constexpr uint32_t overlap{1};
  const std::string filename{"fractal.dat"};

  std::ofstream file(filename, std::ios::binary);
  file.write((char*)&totalSize, sizeof(totalSize));
  file.write((char*)&tileSize, sizeof(tileSize));

  Fractal f(tileSize+2*overlap,tileSize+2*overlap,
            totalSize,totalSize,0,0,selectOpenCLDevice());
  
  std::cout << "Starting fractal computation ... " << std::flush;
  auto t1 = Clock::now();

  std::vector<uint8_t> tile(size_t(tileSize)*size_t(tileSize)*4);
  for (uint32_t tileY = 0; tileY < totalSize/tileSize; ++tileY) {
    for (uint32_t tileX = 0; tileX < totalSize/tileSize; ++tileX) {
      f.setOffset(int64_t(tileX)*int64_t(tileSize)-int64_t(overlap),
                  int64_t(tileY)*int64_t(tileSize)-int64_t(overlap));
      f.compute();
      applyTransferFunction(f.getData(), tile);
      /*
      std::stringstream bmpFilename;
      bmpFilename << "tile_" << 0 << "_" << tileX << "_" << tileY << ".bmp";
      BMP::save(bmpFilename.str(), tileSize+2*overlap, tileSize+2*overlap, tile, 4);
      */
      file.write((char*)tile.data(), std::streamsize(tile.size()));
    }
  }
  
  
  std::vector<uint8_t> tileA(size_t(tileSize)*size_t(tileSize)*4);
  std::vector<uint8_t> tileB(size_t(tileSize)*size_t(tileSize)*4);
  std::vector<uint8_t> tileC(size_t(tileSize)*size_t(tileSize)*4);
  std::vector<uint8_t> tileD(size_t(tileSize)*size_t(tileSize)*4);
  
  uint32_t level{1};
  uint32_t levelSize{totalSize/2};
  while (levelSize >= tileSize) {
    for (uint32_t tileY = 0; tileY < levelSize/tileSize; ++tileY) {
      for (uint32_t tileX = 0; tileX < levelSize/tileSize; ++tileX) {
        
        // TODO: read tiles A-D from file
        
        for (uint32_t pos = 0; pos < (tileSize+2*overlap)*(tileSize+2*overlap)*4; ++pos) {
          const uint16_t valA = tileA[pos];
          const uint16_t valB = tileB[pos];
          const uint16_t valC = tileC[pos];
          const uint16_t valD = tileD[pos];
          tile[pos] = uint8_t((valA+valB+valC+valD)/4);
        }
        
        /*
        std::stringstream bmpFilename;
        bmpFilename << "tile_" << level << "_" << tileX << "_" << tileY << ".bmp";
        BMP::save(bmpFilename.str(), tileSize+2*overlap, tileSize+2*overlap, tile, 4);
        */
        file.write((char*)tile.data(), std::streamsize(tile.size()));
      }
    }
    levelSize /= 2;
    ++level;
  }
  
  
  
  auto t2 = Clock::now();
  std::cout << " done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " milliseconds!" << std::endl;
  

  file.close();

  return EXIT_SUCCESS;
}
