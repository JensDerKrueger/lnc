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
    
    std::cout << "\rProcessing level 0 (" << tileY+1 << "/" << inputDim/tileDim << ")" << std::flush;
    
    for (uint32_t tileX = 0; tileX < inputDim/tileDim; ++tileX) {
      f.setOffset(int64_t(tileX)*int64_t(tileDim)-int64_t(overlap),
                  int64_t(tileY)*int64_t(tileDim)-int64_t(overlap));
      f.compute();
      applyTransferFunction(f.getData(), tile);
            
      // DEBUG
      //std::stringstream bmpFilename;
      //bmpFilename << "tile_" << 0 << "_" << tileX << "_" << tileY << ".bmp";
      //BMP::save(bmpFilename.str(), realTileDim, realTileDim, tile, 4);
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
  //std::fill(targetTile.begin(), targetTile.end(), 255);
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
      //std::stringstream bmpFilename;
      //bmpFilename << "tile_" << level << "_" << tileX << "_" << tileY << "-inner.bmp";
      //BMP::save(bmpFilename.str(), realTileDim, realTileDim, targetTile, 4);
      // DEBUG END
      
      file.seekg(pos, file.beg);
      file.write((char*)targetTile.data(), std::streamsize(targetTile.size()));
    }
  }
}

bool MultiresGen::getTile(int8_t offsetX, int8_t offsetY,
                          const TileCoord& targetCoord,
                          std::vector<uint8_t>& tempTile,
                          TilePositions& tilePositions,
                          std::fstream& file) const {
    
  const TileCoord coord{
    uint32_t(int64_t(targetCoord.x)+offsetX),
    uint32_t(int64_t(targetCoord.y)+offsetY),
    targetCoord.l
  };
    
  if (tilePositions.find(coord) == tilePositions.end()) return false;
   
  const std::streampos sourceFilePos = std::streampos(tilePositions[coord]);
  file.seekg(sourceFilePos, file.beg);
  file.read((char*)tempTile.data(), std::streamsize(tempTile.size()));
  
  return true;
}

void MultiresGen::copyRect(std::vector<uint8_t>& source,
                           std::vector<uint8_t>& target,
                           uint32_t sourceX, uint32_t sourceY,
                           uint32_t targetX, uint32_t targetY,
                           uint32_t width, uint32_t height) const {
  for (uint32_t y = 0; y < height; ++y) {
    for (uint32_t x = 0; x < width; ++x) {
      const size_t sourcePos = (x+sourceX+(y+sourceY)*realTileDim)*4;
      const size_t targetPos = (x+targetX+(y+targetY)*realTileDim)*4;
      for (uint32_t c = 0; c < 4; ++c) {
        target[targetPos+c] = source[sourcePos+c];
      }
    }
  }
}

void MultiresGen::repeatLine(std::vector<uint8_t>& data,
                             uint32_t sourceX, uint32_t sourceY,
                             uint32_t targetX, uint32_t targetY,
                             uint32_t mulX, uint32_t mulY,
                             uint32_t width, uint32_t height) const {
  for (uint32_t y = 0; y < height; ++y) {
    for (uint32_t x = 0; x < width; ++x) {
      const size_t sourcePos = (x*mulX+sourceX+(y*mulY+sourceY)*realTileDim)*4;
      const size_t targetPos = (x+targetX+(y+targetY)*realTileDim)*4;
      for (uint32_t c = 0; c < 4; ++c) {
        data[targetPos+c] = data[sourcePos+c];
      }
    }
  }
}

void MultiresGen::topTileFill(std::fstream &file, const TileCoord &targetCoord,
                          std::vector<uint8_t> &targetTile, std::vector<uint8_t> &tempTile,
                          TilePositions &tilePositions) const {
  if (getTile(0, 1, targetCoord, tempTile, tilePositions, file)) {
    copyRect(tempTile,targetTile,
             (overlap+1)/2, overlap+(overlap+1)/2,
             (overlap+1)/2, realTileDim-(overlap+1)/2,
             realTileDim-((overlap+1)/2)*2, (overlap+1)/2);
  } else {
    repeatLine(targetTile,
               (overlap+1)/2, realTileDim-(overlap+1)/2-1,
               (overlap+1)/2, realTileDim-(overlap+1)/2,
               1, 0,
               realTileDim-((overlap+1)/2)*2, (overlap+1)/2);
  }
}

void MultiresGen::rightTileFill(std::fstream &file, const TileCoord &targetCoord,
                                std::vector<uint8_t> &targetTile, std::vector<uint8_t> &tempTile,
                                TilePositions &tilePositions) const {
  if (getTile(1, 0, targetCoord, tempTile, tilePositions, file)) {
    copyRect(tempTile,targetTile,
             overlap+(overlap+1)/2, (overlap+1)/2,
             realTileDim-(overlap+1)/2, (overlap+1)/2,
             (overlap+1)/2,realTileDim-((overlap+1)/2)*2);
  } else {
    repeatLine(targetTile,
               realTileDim-(overlap+1)/2-1, (overlap+1)/2,
               realTileDim-(overlap+1)/2, (overlap+1)/2,
               0, 1,
               (overlap+1)/2, realTileDim-((overlap+1)/2)*2);
  }
}

void MultiresGen::topRightTileFill(std::fstream &file, const TileCoord &targetCoord,
                                   std::vector<uint8_t> &targetTile, std::vector<uint8_t> &tempTile,
                                   TilePositions &tilePositions) const {
  if (getTile(1, 1, targetCoord, tempTile, tilePositions, file)) {
    copyRect(tempTile,targetTile,
             overlap+(overlap+1)/2, overlap+(overlap+1)/2,
             realTileDim-(overlap+1)/2, realTileDim-(overlap+1)/2,
             (overlap+1)/2,(overlap+1)/2);
  } else {
    repeatLine(targetTile,
               realTileDim-(overlap+1)/2-1, realTileDim-(overlap+1)/2-1,
               realTileDim-(overlap+1)/2, realTileDim-(overlap+1)/2,
               0,0,
               (overlap+1)/2,(overlap+1)/2);
  }
}

void MultiresGen::  leftTileFill(std::fstream &file, const TileCoord &targetCoord,
                                 std::vector<uint8_t> &targetTile, std::vector<uint8_t> &tempTile,
                                 TilePositions &tilePositions) const {
  if (getTile(-1, 0, targetCoord, tempTile, tilePositions, file)) {
    copyRect(tempTile,targetTile,
             realTileDim-(overlap*2), 0,
             0, 0,
             (overlap+1)/2,realTileDim);
    
  } else {
    repeatLine(targetTile,
               (overlap+1)/2, 0,
               0, 0,
               0, 1,
               (overlap+1)/2,realTileDim);
  }
}

void MultiresGen::bottomTileFill(std::fstream &file, const TileCoord &targetCoord,
                                 std::vector<uint8_t> &targetTile, std::vector<uint8_t> &tempTile,
                                 TilePositions &tilePositions) const {
  if (getTile(0, -1, targetCoord, tempTile, tilePositions, file)) {
    copyRect(tempTile,targetTile,
             0, realTileDim-(overlap*2),
             0, 0,
             realTileDim, (overlap+1)/2);
  } else {
    repeatLine(targetTile,
               0, (overlap+1)/2,
               0, 0,
               1, 0,
               realTileDim, (overlap+1)/2);
  }
}

void MultiresGen::fillOverlap(const TileCoord& targetCoord,
                              std::vector<uint8_t>& tempTile,
                              std::vector<uint8_t>& targetTile,
                              TilePositions& tilePositions,
                              std::fstream& file) const {
  topTileFill(file, targetCoord, targetTile, tempTile, tilePositions);
  rightTileFill(file, targetCoord, targetTile, tempTile, tilePositions);
  topRightTileFill(file, targetCoord, targetTile, tempTile, tilePositions);
  leftTileFill(file, targetCoord, targetTile, tempTile, tilePositions);
  bottomTileFill(file, targetCoord, targetTile, tempTile, tilePositions);
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
      // std::stringstream bmpFilename;
      // bmpFilename << "tile_" << level << "_" << tileX << "_" << tileY << "-complete.bmp";
      // BMP::save(bmpFilename.str(), realTileDim, realTileDim, targetTile, 4);
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
    std::cout << "\rProcessing level " << level;
    
    generateInnerTilesOfLevel(level, levelSize, tempTile,
                              targetTile, tilePositions, file);
    fillOverlapOfLevel(level, levelSize, tempTile,
                       targetTile, tilePositions, file);
    levelSize /= 2;
    level++;
  }
  std::cout << std::endl;
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
