#include "DICOMDirParser.h"

#include <filesystem>
#include <cmath>

DICOMDirParser::DICOMDirParser(const std::string& directory) {
  sortIntoStacks(scanFiles(directory));
}

std::vector<DICOMFile> DICOMDirParser::scanFiles(const std::string& directory) const {
  std::vector<DICOMFile> files;
  for (auto& p: std::filesystem::directory_iterator(directory)) {
    try {
      DICOMFile dicomFile{p.path()};
      files.push_back(dicomFile);
    } catch (const DICOMFileException& e) {
      // std::cout << "Unable to read " << p.path() << " " << e.what() << std::endl;
    }
  }
  return files;
}

static bool sortByFirst( const std::vector<DICOMFile>& a, const std::vector<DICOMFile>& b ) {
  return a[0].stackLessCompare(b[0]);
}

static bool sortByDepth( const DICOMFile& a, const DICOMFile& b ) {
  return a.depthLessCompare(b);
}


void DICOMDirParser::sortIntoStacks(const std::vector<DICOMFile>& files) {
  
  // group files into stacks
  for (size_t i = 0; i<files.size(); i++) {
    bool bFoundMatch = false;
    for (size_t j = 0; j<stacks.size(); j++) {
      if (files[i].match(stacks[j][0])) {
        stacks[j].push_back(files[i]);
        bFoundMatch = true;
        break;
      }
    }
    if (!bFoundMatch) {
      stacks.push_back(std::vector<DICOMFile>{files[i]});
    }
  }

  // sort within stack by patient position
  for (size_t i = 0; i<stacks.size(); i++) {
    sort( stacks[i].begin( ), stacks[i].end( ), sortByDepth);
  }

  // sort stacks by sequence number
  sort( stacks.begin( ), stacks.end( ), sortByFirst);
  
  // fix Z aspect ratio - which is broken in many DICOMs - using the patient position
  for (size_t i = 0; i<stacks.size(); i++) {
    if (stacks[i].size() < 2) continue;
    DCMVec3 dir{fabs(stacks[i][1].getPatientPosition().x -
                  stacks[i][0].getPatientPosition().x),
             fabs(stacks[i][1].getPatientPosition().y -
                  stacks[i][0].getPatientPosition().y),
             fabs(stacks[i][1].getPatientPosition().z -
                  stacks[i][0].getPatientPosition().z)};

    float fZDistance = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);

    if (fZDistance != 0) {
      for (size_t j = 0; j<stacks[i].size(); j++) {
        stacks[i][j].correctZAspect(fZDistance);
      }
    }
  }


}
