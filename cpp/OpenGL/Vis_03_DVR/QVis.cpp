#include <fstream>
#include <iterator>
#include <algorithm>
#include <filesystem>

#include "QVis.h"

QVis::QVis(const std::string& filename) {
  load(filename);
}

void QVis::load(const std::string& filename) {
  std::ifstream datfile(filename);
  if (!datfile) throw QVisFileException{std::string("Unable to read file ")+filename};
  
  bool needsConversion{false};
  
  std::filesystem::path p{filename};
  
  std::string rawFilename;
  std::string line;
  while (std::getline(datfile, line)) {
    QVisDatLine l{line};
    
    if (l.id == "objectfilename") {
      if (std::string(p.parent_path()).empty())
        rawFilename = l.value;
      else
        rawFilename = std::string(p.parent_path()) + "/" + l.value;
    } else if (l.id == "resolution") {
      std::vector<std::string> t = tokenize(l.value);
      if (t.size() != 3) throw QVisFileException{"invalid resolution tag"};
      try {
        volume.width  = size_t(stoi(t[0]));
        volume.height = size_t(stoi(t[1]));
        volume.depth  = size_t(stoi(t[2]));
      } catch (const std::invalid_argument&) {
        throw QVisFileException{"invalid resolution tag"};
      }
    }
    else if (l.id == "slicethickness") {
      std::vector<std::string> t = tokenize(l.value);
      if (t.size() != 3) throw QVisFileException{"invalid slicethickness tag"};
      try {
        volume.scale = Vec3{stof(t[0]), stof(t[1]), stof(t[2])};
        volume.normalizeScale();
      } catch (const std::invalid_argument&) {
        throw QVisFileException{"invalid slicethickness tag"};
      }
    }
    else if (l.id == "format") {
      if(l.value != "char" &&
         l.value != "uchar" &&
         l.value != "byte") {
        needsConversion = true;
      }
    }
    else if (l.id == "endianess") {
      if(l.value != "little") {
        throw QVisFileException{"only little endian data supported by this mini-reader"};
      }
    }
  }
  
  datfile.close();
  
  if (rawFilename.empty())
    throw QVisFileException{"object filename not found"};
  
  std::ifstream rawFile( rawFilename, std::ios::binary );  
  
  if (needsConversion) {
    // if it's not 8bit, we assume 16bit
    std::vector<uint16_t> data(volume.width*volume.height*volume.depth);
    rawFile.read((char*)data.data(), std::streamsize(volume.width*volume.height*volume.depth*2));
    uint16_t minVal = data[0], maxVal = data[0];
    
    for (uint16_t val : data ) {
      minVal = std::min(minVal, val);
      maxVal = std::max(maxVal, val);
    }
    volume.data.resize(data.size());
    
    for (size_t i = 0;i<data.size();++i) {
      volume.data[i] = uint8_t(((data[i]-minVal)*255) / (1+maxVal-minVal));
    }
    
  } else {
    volume.data = std::vector<uint8_t>(std::istreambuf_iterator<char>(rawFile), {});
  }
  rawFile.close();

  volume.computeNormals();
}

std::vector<std::string> QVis::tokenize(const std::string& str) const {
  std::vector<std::string> strElements;
  std::string buf;
  std::stringstream ss(str);
  while (ss >> buf) strElements.push_back(buf);

  return strElements;
}

QVisDatLine::QVisDatLine(const std::string input) {
  std::size_t found = input.find_first_of(":");
  if (found==std::string::npos) return;
  
  id    = input.substr(0,found);
  value = input.substr(found+1);
  
  trim(id);
  trim(value);

  transform(id.begin(), id.end(), id.begin(), tolower);
  transform(value.begin(), value.end(), value.begin(), tolower);
}

void QVisDatLine::ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

void QVisDatLine::rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void QVisDatLine::trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}
