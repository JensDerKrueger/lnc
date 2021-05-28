#pragma once

#include <string>
#include <vector>

#include "DICOMFile.h"

class DICOMDirParser {
public:
  DICOMDirParser(const std::string& directory);
  
  size_t getVolumeCount() const {
    return stacks.size();
  }
  
  DCMVec3 getVolmeAspect(size_t i) const {
    return stacks[i][0].getAspect();
  }

  DCMVec3ui getVolmeSize(size_t i) const {
    DCMVec3ui size{stacks[i][0].getSize()};
    size.z = uint32_t(stacks[i].size());
    return size;
  }

  uint32_t getComponentCount(size_t i) const {
    return stacks[i][0].getComponentCount();
  }

  uint32_t getAllocated(size_t i) const {
    return stacks[i][0].getAllocated();
  }
  
  std::vector<uint8_t> getRawData(size_t i) const {
    size_t totalSize{0};
    for (size_t j = 0;j<stacks[i].size();++j) {
      totalSize += stacks[i][j].getRawDataSize();
    }
    std::vector<uint8_t> result(totalSize);
    
    uint32_t offset{0};
    for (size_t j = 0;j<stacks[i].size();++j) {
      std::vector<uint8_t> currentData = stacks[i][j].getData();
      std::copy(currentData.begin(), currentData.end(), result.begin()+offset);
      offset += stacks[i][j].getRawDataSize();
    }
    return result;
  }



private:
  std::vector<std::vector<DICOMFile>> stacks;
    
  std::vector<DICOMFile> scanFiles(const std::string& directory) const ;
  void sortIntoStacks(const std::vector<DICOMFile>& files);

};
