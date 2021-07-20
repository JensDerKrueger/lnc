#include "Compression.h"


namespace Compression {

  void huffmanDecode(const std::string& sourceFilename,
                     const std::string& targetFilename) {
    std::ifstream sourceFile(sourceFilename, std::ios::binary);
    std::ofstream targetFile(targetFilename, std::ios::binary);
    const std::vector<char> source = std::vector<char>((std::istreambuf_iterator<char>(sourceFile)),
                                                         std::istreambuf_iterator<char>());
    std::vector<char> target = huffmanDecode<char>(source);
    targetFile.write((char*)target.data(), long(target.size()));
  }

  void huffmanEncode(const std::string& sourceFilename, const std::string& targetFilename) {
    std::ifstream sourceFile(sourceFilename, std::ios::binary);
    std::ofstream targetFile(targetFilename, std::ios::binary);
    const std::vector<char> source = std::vector<char>((std::istreambuf_iterator<char>(sourceFile)),
                                                             std::istreambuf_iterator<char>());
    const std::vector<char> target = huffmanEncode<char>(source);
    targetFile.write((char*)target.data(), long(target.size()));
  }

  std::vector<bool> intToPath(uint32_t currentIndex, uint32_t length) {
    std::vector<bool> path;
    while (currentIndex > 0) {
      path.push_back(currentIndex % 2);
      currentIndex /= 2;
    }
    std::reverse(path.begin(), path.end());
    for (size_t i = path.size();i<length;++i) {
      path.push_back(false);
    }
    return path;
  }

  uint32_t pathToInt(std::vector<bool> path) {
    uint32_t result{0};
    for (const bool bit : path) {
      result = (result<<1) + bit;
    }
    return result;
  }
  
};
