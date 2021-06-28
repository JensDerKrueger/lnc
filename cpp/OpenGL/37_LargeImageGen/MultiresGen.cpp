#include <iostream>
#include <sstream>

#include <bmp.h>

#include "MultiresGen.h"
#include "Fractal.h"


MultiresGen::MultiresGen(uint32_t inputDim, uint32_t tileDim, uint32_t overlap) :
inputDim(inputDim),
tileDim(tileDim),
overlap(overlap),
realTileDim(tileDim+2*overlap),
totalTileSize(size_t(realTileDim)*size_t(realTileDim)*4)
{}

std::streampos MultiresGen::generateHeader(std::fstream& file) const {
  file.write((char*)&inputDim, sizeof(inputDim));
  file.write((char*)&tileDim, sizeof(tileDim));
  file.write((char*)&overlap, sizeof(overlap));
  
  std::streampos tilePositionsOffsetPos = file.tellg();
  uint64_t tilePositionsOffset = 0; // dummy value, real value is inserted at the end
  file.write((char*)&tilePositionsOffset, sizeof(tilePositionsOffset));
  return tilePositionsOffsetPos;
}

void MultiresGen::generateLevelZero(TilePositions& tilePositions,
                                    cl_device_id dev,
                                    std::fstream& file) const {
  std::cout << "Starting fractal computation ... " << std::endl;

  Fractal f(realTileDim,realTileDim,inputDim,inputDim,0,0,dev);
  
  std::vector<uint8_t> tile(totalTileSize);
  for (uint32_t tileY = 0; tileY < inputDim/tileDim; ++tileY) {
    
    std::cout << "\r" << tileY << "/" << inputDim/tileDim << std::flush;
    
    for (uint32_t tileX = 0; tileX < inputDim/tileDim; ++tileX) {
      f.setOffset(int64_t(tileX)*int64_t(tileDim)-int64_t(overlap),
                  int64_t(tileY)*int64_t(tileDim)-int64_t(overlap));
      f.compute();
      applyTransferFunction(f.getData(), tile);
            
      // DEBUG
      std::stringstream bmpFilename;
      bmpFilename << "tile_" << 0 << "_" << tileX << "_" << tileY << ".bmp";
      BMP::save(bmpFilename.str(), realTileDim, realTileDim, tile, 4);
      // DEBUG END
           
      tilePositions[{tileX,tileY,0}] = int64_t(file.tellg());
      file.write((char*)tile.data(), std::streamsize(tile.size()));
    }
  }
  std::cout << std::endl;
}

void MultiresGen::innerAverage(uint8_t offsetX, uint8_t offsetY,
                               const TileCoord& targetCoord,
                               std::vector<uint8_t>& tempTile,
                               std::vector<uint8_t>& targetTile,
                               TilePositions& tilePositions,
                               std::fstream& file) const {
  
  const TileCoord coord{
    targetCoord.x*2+offsetX,
    targetCoord.y*2+offsetY,
    targetCoord.l-1
  };
  const std::streampos sourceFilePos = std::streampos(tilePositions[coord]);
  file.seekg(sourceFilePos, file.beg);
  file.read((char*)tempTile.data(), std::streamsize(tempTile.size()));
  
  const size_t targetOffsetX = (1-offsetX) * (overlap+1)/2 + offsetX*realTileDim/2;
  const size_t targetOffsetY = (1-offsetY) * (overlap+1)/2 + offsetY*realTileDim/2;

  const size_t sourceOffsetX = offsetX*overlap;
  const size_t sourceOffsetY = offsetY*overlap;
  
  uint32_t end = (realTileDim-overlap)/2;
  
  for (uint32_t y = 0; y < end; ++y) {
    for (uint32_t x = 0; x < end; ++x) {
      size_t targetPos =  (x+targetOffsetX+(y+targetOffsetY)*realTileDim)*4;
      size_t sourcePosA = (x*2+0+sourceOffsetX+(y*2+0+sourceOffsetY)*realTileDim)*4;
      size_t sourcePosB = (x*2+0+sourceOffsetX+(y*2+1+sourceOffsetY)*realTileDim)*4;
      size_t sourcePosC = (x*2+1+sourceOffsetX+(y*2+0+sourceOffsetY)*realTileDim)*4;
      size_t sourcePosD = (x*2+1+sourceOffsetX+(y*2+1+sourceOffsetY)*realTileDim)*4;
      
      for (uint32_t c = 0; c < 4; ++c) {
        const uint16_t valA = tempTile[sourcePosA++];
        const uint16_t valB = tempTile[sourcePosB++];
        const uint16_t valC = tempTile[sourcePosC++];
        const uint16_t valD = tempTile[sourcePosD++];
        targetTile[targetPos++] = uint8_t((valA+valB+valC+valD)/4);
      }
    }
  }
}


void MultiresGen::innerAverage(const TileCoord& targetCoord,
                               std::vector<uint8_t>& tempTile,
                               std::vector<uint8_t>& targetTile,
                               TilePositions& tilePositions,
                               std::fstream& file) const {
  
  // DEBUG
  std::fill(targetTile.begin(), targetTile.end(), 255);
  // DEBUG END
  
  innerAverage(0,0, targetCoord, tempTile, targetTile, tilePositions, file);
  innerAverage(1,0, targetCoord, tempTile, targetTile, tilePositions, file);
  innerAverage(0,1, targetCoord, tempTile, targetTile, tilePositions, file);
  innerAverage(1,1, targetCoord, tempTile, targetTile, tilePositions, file);
}

void MultiresGen::generateInnerTilesOfLevel(uint32_t level, uint32_t levelSize,
                                            std::vector<uint8_t>& tempTile,
                                            std::vector<uint8_t>& targetTile,
                                            TilePositions& tilePositions,
                                            std::fstream& file) const {
  for (uint32_t tileY = 0; tileY < levelSize/tileDim; ++tileY) {
    for (uint32_t tileX = 0; tileX < levelSize/tileDim; ++tileX) {
      std::streampos pos = file.tellg();
      
      const TileCoord coord{tileX,tileY,level};
      tilePositions[coord] = int64_t(pos);

      innerAverage(coord, tempTile, targetTile, tilePositions, file);
      
      // DEBUG
      std::stringstream bmpFilename;
      bmpFilename << "tile_" << level << "_" << tileX << "_" << tileY << "-inner.bmp";
      BMP::save(bmpFilename.str(), realTileDim, realTileDim, targetTile, 4);
      // DEBUG END
      
      file.seekg(pos, file.beg);
      file.write((char*)targetTile.data(), std::streamsize(targetTile.size()));
    }
  }
}

void MultiresGen::fillOverlap(const TileCoord& targetCoord,
                              std::vector<uint8_t>& tempTile,
                              std::vector<uint8_t>& targetTile,
                              TilePositions& tilePositions,
                              std::fstream& file) const {
}

void MultiresGen::fillOverlapOfLevel(uint32_t level, uint32_t levelSize,
                                     std::vector<uint8_t>& tempTile,
                                     std::vector<uint8_t>& targetTile,
                                     TilePositions& tilePositions,
                                     std::fstream& file) const {
  
  std::streampos pos = file.tellg();
  for (uint32_t tileY = 0; tileY < levelSize/tileDim; ++tileY) {
    for (uint32_t tileX = 0; tileX < levelSize/tileDim; ++tileX) {
      const TileCoord coord{tileX,tileY,level};
      file.seekg(std::streampos(tilePositions[coord]), file.beg);
      file.read((char*)targetTile.data(), std::streamsize(targetTile.size()));

      fillOverlap(coord, tempTile, targetTile, tilePositions, file);
      
      // DEBUG
      std::stringstream bmpFilename;
      bmpFilename << "tile_" << level << "_" << tileX << "_" << tileY << "-complete.bmp";
      BMP::save(bmpFilename.str(), realTileDim, realTileDim, targetTile, 4);
      // DEBUG END
      
      file.seekg(std::streampos(tilePositions[coord]), file.beg);
      file.write((char*)targetTile.data(), std::streamsize(targetTile.size()));
    }
  }
  
  file.seekg(pos, file.beg);
}

void MultiresGen::generateHierarchy(TilePositions& tilePositions,
                                    std::fstream& file) const {
  std::vector<uint8_t> tempTile(totalTileSize);
  std::vector<uint8_t> targetTile(totalTileSize);
  
  uint32_t level{1};
  uint32_t levelSize{inputDim/2};
  while (levelSize >= tileDim) {
    std::cout << "Processing level " << level << std::endl;
    
    generateInnerTilesOfLevel(level, levelSize, tempTile,
                              targetTile, tilePositions, file);
    fillOverlapOfLevel(level, levelSize, tempTile,
                       targetTile, tilePositions, file);
    levelSize /= 2;
    level++;
  }
}

void MultiresGen::storeTilePositions(const TilePositions& tilePositions,
                                     const std::streampos tilePositionsOffsetPos,
                                     std::fstream& file ) const {
  std::streampos tilePositionsOffset = file.tellg();
  
  uint64_t tileCount = uint64_t(tilePositions.size());
  file.write((char*)&tileCount, sizeof(uint64_t));
  for (const auto& tilePosition : tilePositions) {
    file.write((char*)&tilePosition.first.x, sizeof(tilePosition.first.x));
    file.write((char*)&tilePosition.first.y, sizeof(tilePosition.first.y));
    file.write((char*)&tilePosition.first.l, sizeof(tilePosition.first.l));
    file.write((char*)&tilePosition.second, sizeof(tilePosition.second));
  }
  file.seekg(tilePositionsOffsetPos, file.beg);
  file.write((char*)&tilePositionsOffset, sizeof(tilePositionsOffset));
}

void MultiresGen::generate(cl_device_id dev, const std::string& filename) const {
  std::fstream file(filename, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
  
  
  TilePositions tilePositions;
  
  const std::streampos tilePositionsOffsetPos = generateHeader(file);
  generateLevelZero(tilePositions, dev, file);
  generateHierarchy(tilePositions, file);
  storeTilePositions(tilePositions, tilePositionsOffsetPos, file);
  
  file.close();
}

Vec3t<uint8_t> MultiresGen::applyTransferFunction(uint8_t input) {
  return {input, uint8_t(input*3), uint8_t(input*25)};
}

void MultiresGen::applyTransferFunction(const std::vector<uint8_t>& inputData,
                                        std::vector<uint8_t>& outputImage) {
  for (size_t i = 0;i<inputData.size();++i) {
    const Vec3t<uint8_t> color = applyTransferFunction(inputData[i]);
    outputImage[i*4+0] = color.r;
    outputImage[i*4+1] = color.g;
    outputImage[i*4+2] = color.b;
    outputImage[i*4+3] = 255;
  }
}
