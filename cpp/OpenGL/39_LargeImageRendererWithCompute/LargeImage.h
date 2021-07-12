#pragma once

// set this define to completely disable reading data from files
// and just use the mandelbrot set computed on the fly
// #define COMPUTE_ONLY

#include <string>
#include <exception>
#include <fstream>
#include <map>
#include <memory>

#include <GLTexture2D.h>
#include <Image.h>

#include <Fractal.h>


struct TileCoord {
  uint32_t x;
  uint32_t y;
  uint32_t l;

  const bool operator < (const TileCoord& other) const {
    return (l < other.l) ||
           (l == other.l && y < other.y) ||
           (l == other.l && y == other.y && x < other.x);
  }
};
using TilePositions = std::map<TileCoord, int64_t>;


class Cache {
public:
  Cache(size_t cacheSize);
  std::shared_ptr<GLTexture2D> getTile(const TileCoord& tileCoord);
  void addTile(const TileCoord& tileCoord, std::shared_ptr<GLTexture2D> tex);
  void clear();

private:
  size_t cacheSize;
  uint64_t now;
  std::map<TileCoord, std::pair<std::shared_ptr<GLTexture2D>, uint64_t>> data;
  void removeOldest();
};

class LargeImage {
public:
  LargeImage(const std::string& filename, size_t cacheSize);
  std::shared_ptr<GLTexture2D> getTile(const TileCoord& tileCoord);

  uint32_t getLevelCount() const;
  uint32_t getLevelTiles(uint32_t l) const;
  uint32_t getInputDim() const;
  uint32_t getTileDim() const;
  uint32_t getOverlap() const;
  uint32_t getRealTileDim() const;
  
  bool getComputeFractal() const;
  void computeFractal(bool compute);

private:
  bool compute{false};
  std::shared_ptr<Fractal> f{nullptr};
  Cache cache;
  std::fstream file;
  uint32_t inputDim;
  uint32_t tileDim;
  uint32_t overlap;
  uint32_t realTileDim;
  size_t totalTileSize;
  TilePositions tilePositions;
  std::vector<uint8_t> tempBuffer;
  std::vector<uint32_t> levelLayout;

  void load(const std::string& filename);
  void loadTilePositions();

};
