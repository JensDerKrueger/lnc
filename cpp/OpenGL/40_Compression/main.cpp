#include <iostream>
#include <fstream>
#include <algorithm>

#include <math.h>

#include "Compression.h"

constexpr uint32_t windowBits = 15;
constexpr uint32_t lookAheadBits = 8;
constexpr uint32_t windowSize = 1<<windowBits;
constexpr uint32_t lookAheadSize = 1<<lookAheadBits;
constexpr uint32_t minSequenceLength = 3;


template <typename T>
T quickLog(T n) {
  T log{0};
  while (n >>= 1) ++log;
  return log;
}

struct DeflateCode {
  uint16_t code;
  uint8_t lengthBitsLength;
  uint16_t lengthBits;
  uint8_t posCode;
  uint8_t posBitsLength;
  uint16_t posBits;

  void serialize(Compression::OutputBitBuffer& buffer,
                 const Compression::Codes<uint16_t>& codeCodes,
                 const Compression::Codes<uint8_t>& posCodes) const {
    
    buffer.addBits(codeCodes.at(code));
    if (code > 256) {
      buffer.add<uint16_t>(lengthBits, lengthBitsLength);
      buffer.addBits(posCodes.at(posCode));
      buffer.add<uint16_t>(posBits, posBitsLength);
    }
  }
};

class LZSSElement {
public:
  uint16_t position;
  uint16_t length;
  char data;
  
  LZSSElement(uint16_t position=0, uint16_t length=0, char data=0) :
    position(position),
    length(length),
    data(data)
  {}
  
  
  DeflateCode toDeflateCode() const {
    if (length == 0) return {uint8_t(data), 0, 0, 0, 0, 0};
    
    uint16_t code;
    uint8_t extraLengthBitCount;
    uint16_t extraLengthBits;
    if (length == 258) {
      code = 285;
      extraLengthBitCount = 0;
      extraLengthBits = 0;
    } else {
      const uint16_t reducedLength = length-3;
      const uint8_t log = uint8_t(quickLog(reducedLength));
      extraLengthBitCount = reducedLength < 8  ? 0 : log-2;
      code = reducedLength < 8 ? 257+reducedLength : 249 + log*4 + (reducedLength>>extraLengthBitCount);
      extraLengthBits  = reducedLength & ((1<<extraLengthBitCount)-1);
    }
        
    uint8_t positionCode;
    uint8_t positionExtraBitCount;
    uint16_t positionExtraBits;
    if (position < 5) {
      positionCode = uint8_t(position - 1);
      positionExtraBitCount = 0;
      positionExtraBits = 0;
    } else {
      const uint16_t reducedDist = position-1;
      const uint8_t log = uint8_t(quickLog(reducedDist));
      positionExtraBitCount = log-1;
      positionCode = uint8_t(log*2 + (reducedDist>>positionExtraBitCount)-2);
      positionExtraBits  = reducedDist & ((1<<positionExtraBitCount)-1);
    }
    
    return {code, extraLengthBitCount, extraLengthBits,
            positionCode, positionExtraBitCount, positionExtraBits};
  }
  
  static bool decode(Compression::InputBitBuffer& buffer,
                     const Compression::NodePtr rootCodes,
                     const Compression::NodePtr rootPos,
                     std::vector<char>& output) {
    
    if (rootCodes == nullptr ||rootPos == nullptr) return false;
    
    const uint16_t code = Compression::decodeElement<uint16_t>(buffer, rootCodes);
    if (code < 256) {
      output.push_back(char(code));
      return true;
    }
    
    if (code == 256) return false;
    
    LZSSElement result;
    
    if (code == 285) {
      result.length = 258;
    } else {
      const uint8_t reconstructedBitCount = uint8_t(code < 265 ? 0 : (code - 261)/4);
      const uint8_t bits = buffer.get<uint8_t>(reconstructedBitCount);
      result.length = (code < 265)
                      ? code - 254
                      : uint16_t(1<<(reconstructedBitCount+2))+3 +
                        ((code - 265)%4)*uint16_t(1<<reconstructedBitCount) +
                         bits;
    }

    const uint8_t posCode = Compression::decodeElement<uint8_t>(buffer, rootPos);
    
    if (posCode < 4) {
      result.position = posCode + 1;
    } else {
      const uint8_t reconstructedBitCount = posCode < 2 ? 0 : uint8_t(posCode/2-1);
      const uint16_t bits = buffer.get<uint16_t>(reconstructedBitCount);
      result.position = (posCode%2)*uint16_t(1<<reconstructedBitCount) +
                        uint16_t(2<<reconstructedBitCount)+1 + bits;
    }

    if (result.length) {
      const size_t pos = output.size();
      for (size_t i = 0;i<result.length;++i) {
        const size_t copySource = size_t(pos-result.position) + i%result.position;
        output.push_back(output[copySource]);
      }
    }
    
    return true;
  }

  
  
};

static LZSSElement search(const std::vector<char>& input, size_t pos) {
  LZSSElement e{0,0,input[pos]};
  
  const size_t searchStart{windowSize >= pos ? 0 : pos-(windowSize-1)};
  const size_t searchEnd{pos};
  
  for (size_t searchPos = searchStart; searchPos < searchEnd; ++searchPos) {
    uint16_t length{0};
    const size_t maxLength{pos-searchPos};
    
    while (pos+length < input.size() &&
           input[searchPos+length%maxLength] == input[pos+length] &&
           length < minSequenceLength+lookAheadSize-1) {
      length++;
    }
    
    if (length >= e.length && length >= minSequenceLength) {
      e.data     = 0;
      e.length   = length;
      e.position = uint16_t(pos-searchPos);
    }
  }
    
  return e;
}

static std::vector<char> compress(const std::vector<char>& input) {
  Compression::OutputBitBuffer buffer;
  std::vector<DeflateCode> deflateCodes;
  
  Compression::Histogram<uint16_t> codeHistogram;
  Compression::Histogram<uint8_t> posHistogram;
  
  size_t pos{0};
  while (pos < input.size()) {
    LZSSElement element = search(input, pos);
    
    const DeflateCode deflateCode = element.toDeflateCode();
    Compression::addToHistogram(deflateCode.code, codeHistogram);
    
    if (element.length > 0) {
      Compression::addToHistogram(deflateCode.posCode, posHistogram);
    }
    
    deflateCodes.push_back(deflateCode);
    pos += std::max<size_t>(1,element.length);
  }
  
  Compression::addToHistogram(uint16_t(256), codeHistogram);
  
  const Compression::NodePtr codeRoot = Compression::canonicalize<uint16_t>(Compression::huffmanCoding(codeHistogram));
  const Compression::NodePtr posRoot = Compression::canonicalize<uint8_t>(Compression::huffmanCoding(posHistogram));
    
  const Compression::Codes<uint16_t> codeCodes = Compression::treeToCodes<uint16_t>(codeRoot);
  const Compression::Codes<uint8_t> posCodes = Compression::treeToCodes<uint8_t>(posRoot);
  
  const std::vector<uint32_t> codeCodebook = Compression::toCanonicalEncoding2<uint16_t>(codeRoot);
  const std::vector<uint32_t> posCodebook = Compression::toCanonicalEncoding2<uint8_t>(posRoot);
  
  auto maxElem = std::max_element(codeCodebook.begin(), codeCodebook.end());
  uint8_t bitCount = uint8_t(ceil(log2(*maxElem)));
  
  buffer.add(codeCodebook.size()-257, 5);
  buffer.add(bitCount, 8);
  
  for (const uint32_t sizeEntry : codeCodebook) {
    buffer.add(sizeEntry, bitCount);
  }
  
  maxElem = std::max_element(posCodebook.begin(), posCodebook.end());
  bitCount = uint8_t(ceil(log2(*maxElem)));
  
  buffer.add(posCodebook.size()-1, 5);
  buffer.add(bitCount, 8);
  for (const uint32_t sizeEntry : posCodebook) {
    buffer.add(sizeEntry, bitCount);
  }

  for (const auto& deflateCode : deflateCodes) {
    deflateCode.serialize(buffer, codeCodes, posCodes);
  }
  buffer.addBits(codeCodes.at(256));
  
  return buffer.getData();
}

static void compress(const std::string& sourceFilename, const std::string& targetFilename) {
  std::ifstream sourceFile(sourceFilename, std::ios::binary);
  std::ofstream targetFile(targetFilename, std::ios::binary);
  const std::vector<char> source = std::vector<char>((std::istreambuf_iterator<char>(sourceFile)),
                                                      std::istreambuf_iterator<char>());
  const auto compressed = compress(source);
    
  targetFile.write((char*)compressed.data(), long(compressed.size()));
}


static std::vector<char> decompress(const std::vector<char>& input) {
  std::vector<char> output;
  
  Compression::InputBitBuffer buffer(input);
  
  uint16_t codeCodebookSize = buffer.get<uint16_t>(5) + 257;
  uint8_t codeCodebookBitCount = buffer.get<uint8_t>(8);

  if (codeCodebookSize > 286) {
    throw Compression::Exception("codeCodebookSize invalid");
  }
  
  std::vector<uint32_t> codeCodebook;
  for (uint16_t i = 0;i<codeCodebookSize;++i) {
    codeCodebook.push_back(buffer.get<uint32_t>(codeCodebookBitCount));
  }
    
  uint16_t posCodebookSize = buffer.get<uint16_t>(5) + 1;
  uint8_t posCodebookBitCount = buffer.get<uint8_t>(8);

  if (posCodebookSize > 30) {
    throw Compression::Exception("codeCodebookSize invalid");
  }

  std::vector<uint32_t> posCodebook;
  for (uint16_t i = 0;i<posCodebookSize;++i) {
    posCodebook.push_back(buffer.get<uint32_t>(posCodebookBitCount));
  }

  const Compression::NodePtr rootCodes = Compression::fromCanonicalEncoding2<uint16_t>(codeCodebook);
  const Compression::NodePtr rootPos = Compression::fromCanonicalEncoding2<uint8_t>(posCodebook);
  
  if (!Compression::checkTree<uint16_t>(rootCodes)) {
    throw Compression::Exception("rootPos tree invalid");
  }
  
  if (!Compression::checkTree<uint8_t>(rootPos)) {
    throw Compression::Exception("rootPos tree invalid");
  }
  
  while (LZSSElement::decode(buffer, rootCodes, rootPos, output)) {}
    
  return output;
}

static void decompress(const std::string& sourceFilename,
                       const std::string& targetFilename) {
  std::ifstream sourceFile(sourceFilename, std::ios::binary);
  std::ofstream targetFile(targetFilename, std::ios::binary);
  
  const std::vector<char> source = std::vector<char>((std::istreambuf_iterator<char>(sourceFile)),
                                                      std::istreambuf_iterator<char>());
    
  const std::vector<char> target = decompress(source);
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
