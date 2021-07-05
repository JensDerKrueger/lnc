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

void LargeImage::load(const std::string& filename) {
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
}

std::shared_ptr<GLTexture2D> LargeImage::getTile(const TileCoord& tileCoord) {
 std::shared_ptr<GLTexture2D> result = cache.getTile(tileCoord);
 if (result != nullptr) return result;
  
  const std::streampos sourceFilePos = std::streampos(tilePositions[tileCoord]);
  file.seekg(sourceFilePos, file.beg);
  file.read((char*)tempBuffer.data(), std::streamsize(tempBuffer.size()));
  
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
