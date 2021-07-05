#include "LargeImage.h"

#include <filesystem>
#include <algorithm>
#include <cstdio>

#include <bmp.h>

LargeImage::LargeImage(const std::string& filename, size_t cacheSize):
  cache(cacheSize)
{
  load(filename);
}

static Vec3t<uint8_t> applyTransferFunction(uint8_t input) {
  /*
  std::vector<Vec3t<uint8_t>> colorTable{
    {   0,   7, 100 },
    {  32, 107, 203 },
    { 237, 255, 255 },
    { 255, 170,   0 },
    { 106,  53,   3 }
  };
  
  const float coords = input/255.0f * (colorTable.size()-1);
  const float alpha = coords - floor(coords);
  
  
  const Vec3t<uint8_t> f = colorTable[size_t(floor(coords))];
  const Vec3t<uint8_t> c = colorTable[size_t(ceil(coords))];
  
  
  const float r = (1-alpha) * f.r + alpha * c.r;
  const float g = (1-alpha) * f.g + alpha * c.g;
  const float b = (1-alpha) * f.b + alpha * c.b;
    
  return {uint8_t(r),uint8_t(g),uint8_t(b)};
  */
  return {input, uint8_t(input*3), uint8_t(input*25)};
}

static void applyTransferFunction(const std::vector<uint8_t>& inputData,
                                        std::vector<uint8_t>& outputImage) {
  for (size_t i = 0;i<inputData.size();++i) {
    const Vec3t<uint8_t> color = applyTransferFunction(inputData[i]);
    outputImage[i*4+0] = color.r;
    outputImage[i*4+1] = color.g;
    outputImage[i*4+2] = color.b;
    outputImage[i*4+3] = 255;
  }
}


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

void LargeImage::load(const std::string& filename) {

  /*
   inputDim = 1u<<31u;
   tileDim = 512;
   overlap = 1;
   */
  
  file = std::fstream(filename, std::ios::binary | std::ios::in);

  if (!file.is_open())
    throw std::runtime_error("Failed to open large image");
  
  file.read((char*)&inputDim, sizeof(inputDim));
  file.read((char*)&tileDim, sizeof(tileDim));
  file.read((char*)&overlap, sizeof(overlap));
  
  levelLayout.clear();
  uint32_t levelDim{inputDim};
  while (levelDim >= tileDim) {
    levelLayout.push_back(levelDim/tileDim);
    levelDim /= 2;
  }
  
  realTileDim = tileDim+2*overlap;
  totalTileSize = size_t(realTileDim)*size_t(realTileDim)*4;
  
  tempBuffer.resize(totalTileSize);
  
  uint64_t tilePositionsOffset;
  file.read((char*)&tilePositionsOffset, sizeof(tilePositionsOffset));
  file.seekg(int64_t(tilePositionsOffset), file.beg);
  
  if (tilePositionsOffset == 0)
    throw std::runtime_error("Failed to load large image (incomplete data)");

  loadTilePositions();

  f = std::make_shared<Fractal>(realTileDim,realTileDim,inputDim,inputDim,0,0,selectOpenCLDevice());
}


std::shared_ptr<GLTexture2D> LargeImage::getTile(const TileCoord& tileCoord) {
  std::shared_ptr<GLTexture2D> result = cache.getTile(tileCoord);
  if (result != nullptr) return result;
  
  if (compute) {
    f->setResolution(inputDim/(1<<tileCoord.l), inputDim/(1<<tileCoord.l));
    f->setOffset(int64_t(tileCoord.x)*int64_t(tileDim)-int64_t(overlap),
                 int64_t(tileCoord.y)*int64_t(tileDim)-int64_t(overlap));
    f->compute();
    applyTransferFunction(f->getData(), tempBuffer);
  } else {
    const std::streampos sourceFilePos = std::streampos(tilePositions[tileCoord]);
    file.seekg(sourceFilePos, file.beg);
    file.read((char*)tempBuffer.data(), std::streamsize(tempBuffer.size()));
  }
   
  std::shared_ptr<GLTexture2D> tex = std::make_shared<GLTexture2D>(Image(realTileDim, realTileDim, 4, tempBuffer));

  cache.addTile(tileCoord, tex);
  return tex;
}

void LargeImage::loadTilePositions() {
  tilePositions.clear();
  uint64_t tileCount;
  file.read((char*)&tileCount, sizeof(uint64_t));

  int64_t offset;
  uint32_t x,y,l;
  for (uint64_t i = 0;i<tileCount;++i) {
    file.read((char*)&x, sizeof(x));
    file.read((char*)&y, sizeof(y));
    file.read((char*)&l, sizeof(l));
    file.read((char*)&offset, sizeof(offset));
    tilePositions[{x,y,l}] = offset;
  }
}

uint32_t LargeImage::getLevelCount() const {
  return uint32_t(levelLayout.size());;
}

uint32_t LargeImage::getLevelTiles(uint32_t l) const {
  return levelLayout[l];
}

uint32_t LargeImage::getInputDim() const {
  return inputDim;
}

uint32_t LargeImage::getTileDim() const {
  return tileDim;
}

uint32_t LargeImage::getRealTileDim() const {
  return realTileDim;
}

uint32_t LargeImage::getOverlap() const {
  return overlap;
}

void LargeImage::computeFractal(bool compute) {
  if (this->compute != compute) {
    cache.clear();
    this->compute = compute;
  }
}

bool LargeImage::getComputeFractal() const {
  return compute;
}

Cache::Cache(size_t cacheSize) :
  cacheSize{cacheSize},
  now{0}
{
}

std::shared_ptr<GLTexture2D> Cache::getTile(const TileCoord& tileCoord) {
  if (data.find(tileCoord) != data.end() ) {
    data[tileCoord].second = now++;
    return data[tileCoord].first;
  } else {
    return nullptr;
  }
}

void Cache::addTile(const TileCoord& tileCoord, std::shared_ptr<GLTexture2D> tex) {
  if (data.size() >= cacheSize)
    removeOldest();
  data[tileCoord] = std::make_pair(tex, now++);
}

void Cache::removeOldest() {
  uint64_t oldest = now;
  
  for (const auto& tile : data) {
    if (tile.second.second < oldest) {
      oldest = tile.second.second;
    }
  }
  
  for (const auto& tile : data) {
    if (tile.second.second == oldest) {
      data.erase(tile.first);
      break;
    }
  }
}

void Cache::clear() {
  data.clear();
  now = 0;
}
