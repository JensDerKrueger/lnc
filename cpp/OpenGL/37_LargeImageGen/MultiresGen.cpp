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

void MultiresGen::generateHeader(std::fstream& file) const {
  file.write((char*)&inputDim, sizeof(inputDim));
  file.write((char*)&tileDim, sizeof(tileDim));
  file.write((char*)&overlap, sizeof(overlap));
}

void MultiresGen::generateLevelZero(TilePositions& tilePositions,
                                    cl_device_id dev,
                                    std::fstream& file) const {
  
  Fractal f(realTileDim,realTileDim,inputDim,inputDim,0,0,dev);
  
  std::vector<uint8_t> tile(totalTileSize);
  for (uint32_t tileY = 0; tileY < inputDim/tileDim; ++tileY) {
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
}

void MultiresGen::innerAverage(uint32_t offsetX, uint32_t offsetY,
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
      for (uint32_t c = 0; c < 4; ++c) {
        
        const uint16_t valA = tempTile[(x*2+0+sourceOffsetX+(y*2+0+sourceOffsetY)*realTileDim)*4+c];
        const uint16_t valB = tempTile[(x*2+0+sourceOffsetX+(y*2+1+sourceOffsetY)*realTileDim)*4+c];
        const uint16_t valC = tempTile[(x*2+1+sourceOffsetX+(y*2+0+sourceOffsetY)*realTileDim)*4+c];
        const uint16_t valD = tempTile[(x*2+1+sourceOffsetX+(y*2+1+sourceOffsetY)*realTileDim)*4+c];
        
        const size_t targetPos = (x+targetOffsetX+(y+targetOffsetY)*realTileDim)*4+c;
        targetTile[targetPos] = uint8_t((valA+valB+valC+valD)/4);
      }
    }
  }
}


void MultiresGen::innerAverage(const TileCoord& coord,
                               std::vector<uint8_t>& tempTile,
                               std::vector<uint8_t>& targetTile,
                               TilePositions& tilePositions,
                               std::fstream& file) const {
  
  // DEBUG
  std::fill(targetTile.begin(), targetTile.end(), 255);
  // DEBUG END
  
  innerAverage(0,0, coord, tempTile, targetTile, tilePositions, file);
  innerAverage(1,0, coord, tempTile, targetTile, tilePositions, file);
  innerAverage(0,1, coord, tempTile, targetTile, tilePositions, file);
  innerAverage(1,1, coord, tempTile, targetTile, tilePositions, file);
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
      bmpFilename << "tile_" << level << "_" << tileX << "_" << tileY << ".bmp";
      BMP::save(bmpFilename.str(), realTileDim, realTileDim, targetTile, 4);
      // DEBUG END
      
      file.seekg(pos, file.beg);
      file.write((char*)targetTile.data(), std::streamsize(targetTile.size()));
    }
  }
}

void MultiresGen::generateHierarchy(TilePositions& tilePositions,
                                    std::fstream& file) const {
  std::vector<uint8_t> tempTile(totalTileSize);
  std::vector<uint8_t> targetTile(totalTileSize);
  
  uint32_t level{1};
  uint32_t levelSize{inputDim/2};
  while (levelSize >= tileDim) {
    generateInnerTilesOfLevel(level, levelSize, tempTile,
                              targetTile, tilePositions, file);
    //fillOverlap();
    levelSize /= 2;
    level++;
  }
}

void MultiresGen::generate(cl_device_id dev, const std::string& filename) const {
  std::fstream file(filename, std::ios::binary | std::ios::in | std::ios::out);
  TilePositions tilePositions;
  
  generateHeader(file);
  generateLevelZero(tilePositions, dev, file);
  generateHierarchy(tilePositions, file);
  
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
