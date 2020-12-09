#include <fstream>
#include <algorithm>

#include "OBJFile.h"

OBJFile::OBJFile(const std::string& filename, bool normalize) {
  Vec3 minVal{};
  Vec3 maxVal{};
  
  std::ifstream f(filename);
  std::string line;
  while (std::getline(f, line)) {
    trim(line);
    if (line.size() < 2) continue;
    
    if (line[0] == 'f') {
      std::vector<std::string> face = tokenize(line,1);
      if (face.size() != 3) continue;
      indices.push_back({fromStr<size_t>(face[0])-1,fromStr<size_t>(face[1])-1,fromStr<size_t>(face[2])-1});
    } else {
      if (line[0] == 'v') {
        if (line[1] == 'n') {
          std::vector<std::string> normal = tokenize(line,2);
          if (normal.size() != 3) continue;
          normals.push_back({fromStr<float>(normal[0]),fromStr<float>(normal[1]),fromStr<float>(normal[2])});
        } else {
          std::vector<std::string> vertex = tokenize(line,1);
          if (vertex.size() != 3) continue;
          Vec3 v{fromStr<float>(vertex[0]),fromStr<float>(vertex[1]),fromStr<float>(vertex[2])};
          
          if (vertices.empty()) {
            minVal = v;
            maxVal = v;
          } else {
            for (size_t i = 0;i<3;++i) {
              minVal[i] = std::min(minVal[i], v[i]);
              maxVal[i] = std::max(maxVal[i], v[i]);
            }
            
          }
          
          vertices.push_back(v);
        }
      }
    }
  }
  f.close();
  
  if (normalize) {
    Vec3 center = (maxVal + minVal)/2.0f;
    float maxSize = std::max(maxVal[0] - minVal[0], std::max(maxVal[1] - minVal[1], maxVal[2] - minVal[2]));

    for (size_t i = 0;i<vertices.size();++i) {
      vertices[i] = (vertices[i] - center) / maxSize;
    }
  }
  
  normals.resize(vertices.size());
  for (const OBJFile::IndexType& triangle : indices) {
    std::array<Vec3, 3> v;
    for (size_t i = 0;i<3;++i) {
      v[0] = Vec3{vertices[triangle[0]][0],vertices[triangle[0]][1],vertices[triangle[0]][2]};
      v[1] = Vec3{vertices[triangle[1]][0],vertices[triangle[1]][1],vertices[triangle[1]][2]};
      v[2] = Vec3{vertices[triangle[2]][0],vertices[triangle[2]][1],vertices[triangle[2]][2]};
      
      Vec3 normal = Vec3::normalize(Vec3::cross(v[1]-v[0], v[2]-v[0]));
      
      normals[triangle[0]] = normal;
      normals[triangle[1]] = normal;
      normals[triangle[2]] = normal;
    }
  }
  
}

void OBJFile::ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

void OBJFile::rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void OBJFile::trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

std::vector<std::string> OBJFile::tokenize(const std::string& str, size_t startpos) {
  std::vector<std::string> strElements;
  std::string buf;
  std::stringstream ss(str.substr(startpos));
  while (ss >> buf) strElements.push_back(buf);
  return strElements;
}
