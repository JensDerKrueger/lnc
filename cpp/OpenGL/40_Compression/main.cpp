#include <iostream>
#include <fstream>

#include "Compression.h"

constexpr uint32_t windowBits = 16;
constexpr uint32_t lookAheadBits = 8;
constexpr uint32_t windowSize = 1<<windowBits;
constexpr uint32_t lookAheadSize = 1<<lookAheadBits;

struct LZSSElement {
  uint16_t position;
  uint8_t length;
  char data;
  
  LZSSElement(uint16_t position=0, uint8_t length=0, char data=0) :
    position(position),
    length(length),
    data(data)
  {}
  
  LZSSElement(Compression::InputBitBuffer& buffer) :
    position(0),
    length(0),
    data(0)
  {
    if (buffer.getBit()) {
      position = buffer.get<decltype(position)>();
      length = buffer.get<decltype(length)>();
    }
    data = buffer.getChar();
  }

  
  void serialize(Compression::OutputBitBuffer& buffer) const {
    if (length == 0) {
      buffer.addBit(0);
      buffer.addChar(data);
    } else {
      buffer.addBit(1);
      buffer.add(position);
      buffer.add(length);
      buffer.addChar(data);
    }
  }
  
  static void decode(Compression::InputBitBuffer& buffer,
                          std::vector<char>& output) {
    LZSSElement e{buffer};
    if (e.length) {
      const size_t pos = output.size();
      for (size_t i = 0;i<e.length;++i) {
        const size_t copySource = size_t(pos-e.position) + i;
        output.push_back(output[copySource]);
      }
    }
    output.push_back(e.data);
  }
  
};

static LZSSElement search(const std::vector<char>& input, size_t pos) {
  LZSSElement e{0,0,input[pos]};
  
  size_t searchStart = windowSize >= pos ? 0 : pos-(windowSize-1);
  size_t searchEnd   = pos;
  
  for (size_t searchPos = searchStart; searchPos < searchEnd; ++searchPos) {
    uint8_t length{0};
    
    while (input[searchPos+length] == input[pos+length] &&
           pos+length < input.size()-1 &&
           length < lookAheadSize-1 &&
           searchPos+length < pos) {
      length++;
    }
    
    if (length > e.length) {
      e.length   = length;
      e.position = uint16_t(pos-searchPos);
      e.data     = input[pos+length];
    }
  }
  
  if (9*e.length < 9+windowBits+lookAheadBits) {
    return {0,0,input[pos]};
  }
  
  return e;
}

static std::pair<std::vector<char>, uint64_t> compress(const std::vector<char>& input) {
  Compression::OutputBitBuffer buffer;
  uint64_t total{0};
  size_t pos{0};
  
  while (pos < input.size()) {
    const LZSSElement element = search(input, pos);
    element.serialize(buffer);
    pos += element.length+1;
    total++;
  }
  
  return {buffer.getData(), total};
}

static void compress(const std::string& sourceFilename, const std::string& targetFilename) {
  std::ifstream sourceFile(sourceFilename, std::ios::binary);
  std::ofstream targetFile(targetFilename, std::ios::binary);
  const std::vector<char> source = std::vector<char>((std::istreambuf_iterator<char>(sourceFile)),
                                                      std::istreambuf_iterator<char>());
  const auto [target, total] = compress(source);
  targetFile.write((char*)&total, sizeof(total));
  targetFile.write((char*)target.data(), long(target.size()));
}


static std::vector<char> decompress(const std::vector<char>& input, uint64_t total) {
  if (input.empty()) return {};
  
  Compression::InputBitBuffer buffer(input);
  std::vector<char> output;
  for (size_t i = 0;i<total;++i) {
    LZSSElement::decode(buffer, output);
  }  
  return output;
}

static void decompress(const std::string& sourceFilename, const std::string& targetFilename) {
  std::ifstream sourceFile(sourceFilename, std::ios::binary);
  std::ofstream targetFile(targetFilename, std::ios::binary);
  
  uint64_t total;
  sourceFile.read((char*)&total, sizeof(total));
  
  const std::vector<char> source = std::vector<char>((std::istreambuf_iterator<char>(sourceFile)),
                                                      std::istreambuf_iterator<char>());
    
  const std::vector<char> target = decompress(source,total);
  targetFile.write((char*)target.data(), long(target.size()));
}

static void showUsage(char** argv) {
  std::cout << "Usage:" << std::endl;
  std::cout << "\t compress a file " << argv[0] << " c input output" << std::endl;
  std::cout << "\t uncompress a file " << argv[0] << " u input output" << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 4) {
    showUsage(argv);
    return EXIT_FAILURE;
  }
  
  if (std::string{argv[1]} == "c")
    compress(argv[2], argv[3]);
  else if (std::string{argv[1]} == "u")
    decompress(argv[2], argv[3]);
  else {
    showUsage(argv);
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
