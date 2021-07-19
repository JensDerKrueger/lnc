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


};
