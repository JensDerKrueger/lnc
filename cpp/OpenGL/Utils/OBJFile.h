#pragma once

#include <vector>
#include <array>
#include <string>
#include <sstream>

#include "Vec3.h"

class OBJFile {
public:
  OBJFile(const std::string& filename, bool normalize=false);
  
  typedef std::array<size_t, 3> IndexType;

  std::vector<IndexType> indices;
  std::vector<Vec3> vertices;
  std::vector<Vec3> normals;
  
private:
  void ltrim(std::string &s);
  void rtrim(std::string &s);
  void trim(std::string &s);
  std::vector<std::string> tokenize(const std::string& str, size_t startpos);

  template <typename T> T fromStr(const std::string& str) {
    T t;
    std::istringstream iss{str};
    iss >> std::dec >> t;
    return t;
  }


};

