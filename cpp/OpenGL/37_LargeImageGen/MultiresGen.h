#pragma once

#include <string>
#include <fstream>
#include <map>

#include <Vec3.h>

#include "OpenClContext.h"

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

class MultiresGen {
public:
  MultiresGen(uint32_t inputDim, uint32_t tileDim, uint32_t overlap);  
  void generate(cl_device_id dev, const std::string& filename) const;

private:
  const uint32_t inputDim;
  const uint32_t tileDim;
  const uint32_t overlap;
  
  const uint32_t realTileDim;
  const size_t totalTileSize;

  std::streampos generateHeader(std::fstream& file) const;
  void generateLevelZero(TilePositions& tilePositions, cl_device_id dev, std::fstream& file) const;
  void generateHierarchy(TilePositions& tilePositions, std::fstream& file) const;
  void generateInnerTilesOfLevel(uint32_t level, uint32_t levelSize,
                                 std::vector<uint8_t>& tempTile,
                                 std::vector<uint8_t>& targetTile,
                                 TilePositions& tilePositions,
                                 std::fstream& file) const;
  
  void innerAverage(const TileCoord& targetCoord,
                    std::vector<uint8_t>& tempTile,
                    std::vector<uint8_t>& targetTile,
                    TilePositions& tilePositions,
                    std::fstream& file) const;
  
  void innerAverage(uint8_t offsetX, uint8_t offsetY,
                    const TileCoord& targetCoord,
                    std::vector<uint8_t>& tempTile,
                    std::vector<uint8_t>& targetTile,
                    TilePositions& tilePositions,
                    std::fstream& file) const;
  
  void fillOverlapOfLevel(uint32_t level, uint32_t levelSize,
                          std::vector<uint8_t>& tempTile,
                          std::vector<uint8_t>& targetTile,
                          TilePositions& tilePositions,
                          std::fstream& file) const;
  void fillOverlap(const TileCoord& targetCoord,
                   std::vector<uint8_t>& tempTile,
                   std::vector<uint8_t>& targetTile,
                   TilePositions& tilePositions,
                   std::fstream& file) const;
  bool getTile(int8_t offsetX, int8_t offsetY,
               const TileCoord& targetCoord,
               std::vector<uint8_t>& tempTile,
               TilePositions& tilePositions,
               std::fstream& file) const;
  
  void storeTilePositions(const TilePositions& tilePositions,
                          const std::streampos tilePositionsOffsetPos,
                          std::fstream& file ) const;


  static Vec3t<uint8_t> applyTransferFunction(uint8_t input);
  static void applyTransferFunction(const std::vector<uint8_t>& inputData,
                                    std::vector<uint8_t>& outputImage);
};
