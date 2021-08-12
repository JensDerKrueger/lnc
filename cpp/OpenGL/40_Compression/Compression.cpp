#include <iostream>
#include <fstream>
#include <queue>
#include <map>
#include <bitset>
#include <memory>
#include <algorithm>
#include <array>
#include <list>
#include <math.h>

#include "Compression.h"

#include <bmp.h>

namespace Compression {

  using Tripple = std::array<char,3>;
  
  static size_t trippleHash(const Tripple& t, size_t s) {
    std::size_t h1 = size_t(t[0]);
    std::size_t h2 = size_t(t[1]);
    std::size_t h3 = size_t(t[2]);
    return (h1 ^ (h2 * s/(256*3)) ^ (2*h3/(256*3)) ) % s;
  }
  
  class HashTable {
  public:
    HashTable(size_t size) {
      data.resize(size);
    }

    void add(const Tripple& t, size_t s) {
      const size_t index = trippleHash(t,data.size());
      data[index].push_front(std::make_pair(t,s));
    }

    const std::list<std::pair<Tripple, size_t>>& get(const Tripple& t) const {
      const size_t index = trippleHash(t,data.size());
      return data[index];
    }
    
    void trim(const Tripple& t, std::list<std::pair<Tripple, size_t>>::const_iterator& pos) {
      const size_t index = trippleHash(t,data.size());
      data[index].erase(pos, data[index].end());
    }

  private:
    std::vector<std::list<std::pair<Tripple, size_t>>> data;
  };

  template <typename T> using Histogram = std::map<T, size_t>;  
  template <typename T> using Codes = std::map<T, std::vector<bool>>;

  struct Node {
    size_t frequency;
    Node(size_t frequency) : frequency{frequency} {}
    virtual ~Node() {}
  };
  using NodePtr = std::shared_ptr<Node>;

  struct InnerNode : Node {
    NodePtr left;
    NodePtr right;
    
    InnerNode() :
    Node{0},
    left{nullptr},
    right{nullptr} {}

    InnerNode(NodePtr left, NodePtr right) :
    Node{left->frequency + right->frequency},
    left{left},
    right{right}
    {}
    
    virtual ~InnerNode() {}
  };
  using InnerNodePtr = std::shared_ptr<InnerNode>;

  template <typename T>
  struct LeafNode : Node {
    T data;
    LeafNode(T data, size_t frequency) :
    Node{frequency},
    data{data}
    {}
    
    virtual ~LeafNode() {}
  };
  template <typename T>
  using LeafNodePtr = std::shared_ptr<LeafNode<T>>;

  struct NodeGreater {
    bool operator() (NodePtr left, NodePtr right) const {
      return left->frequency > right->frequency;
    }
  };
  
  class BitBuffer {
  public:
    BitBuffer() {}
    BitBuffer(const std::vector<char>& data) :
      data{data}
    {
    }
    
    const std::vector<char>& getData() const {
      return data;
    }
    
    
  protected:
    std::vector<char> data;
  };

  class OutputBitBuffer : public BitBuffer {
  public:
    void addBit(bool bit) {
      if (bitInLastByte == 0) {
        data.push_back(0);
      }
      data.back() = data.back() | uint8_t(bit << (7-bitInLastByte));
      bitInLastByte = (bitInLastByte+1)%8;
    }
    
    void addBits(const std::vector<bool>& bits) {
      for (const bool b : bits) addBit(b);
    }
    
    void addChar(char c) {
      if (bitInLastByte == 0) {
        data.push_back(c);
      } else {
        const uint8_t msb = uint8_t(c) >> bitInLastByte;
        const uint8_t lsb = uint8_t(uint8_t(c) << (8-bitInLastByte));
        data.back() = data.back() | msb;
        data.push_back(char(lsb));
      }
    }
    
    template <typename T>
    void add(const T& data) {
      char* p = (char*)&data;
      for (size_t i = 0;i<sizeof(T);++i) {
        addChar(p[i]);
      }
    }

    template <typename T>
    void add(T number, uint8_t bits) {
      for (uint8_t bit = 0;bit<bits;++bit) {
        addBit(number & 1);
        number >>= 1;
      }
    }

  private:
    uint8_t bitInLastByte{0};
  };

  class InputBitBuffer : public BitBuffer {
  public:
    InputBitBuffer(const std::vector<char>& data) :
    BitBuffer{data}
    {
    }

    bool getBit() {
      const bool result = data[currentByte] & (1 << (7-bitInCurrentByte));
      if (bitInCurrentByte == 7) {
        bitInCurrentByte = 0;
        ++currentByte;
      } else {
        ++bitInCurrentByte;
      }
      return result;
    }
    
    char getChar() {
      if (bitInCurrentByte == 0) {
        return char(data[currentByte++]);
      } else {
        const uint8_t msb = uint8_t(data[currentByte++] << bitInCurrentByte);
        const uint8_t lsb = uint8_t(data[currentByte]) >> (8-bitInCurrentByte);
        return char(msb | lsb);
      }
    }
    
    template <typename T>
    T get() {
      T t;
      char* p = (char*)&t;
      for (size_t i = 0;i<sizeof(T);++i) {
        p[i] = getChar();
      }
      return t;
    }
    
    template <typename T>
    T get(uint8_t bits) {
      T number{0};
      T multi{1};
      for (uint8_t bit = 0;bit<bits;++bit) {
        number += T(getBit())*multi;
        multi <<= 1;
      }
      return number;
    }
    
    bool isValid() {
      return currentByte < data.size();
    }

  private:
    uint8_t bitInCurrentByte{0};
    uint64_t currentByte{0};
  };

  template <typename T>
  void addToHistogram(const T& e, Histogram<T>& histogram) {
    if (histogram.find(e) == histogram.end()) {
      histogram[e] = 1;
    } else {
      histogram[e]++;
    }
  }

  template <typename T>
  std::pair<Histogram<T>, uint64_t> buildHistogram(const std::vector<T>& source) {
    Histogram<T> histogram;
    uint64_t total{0};
    for (const T& e : source) {
      addToHistogram<T>(e,histogram);
      ++total;
    }
    return {histogram, total};
  }

  template <typename T>
  void writeCodebook(const NodePtr node, OutputBitBuffer& buffer) {
    const LeafNodePtr<T> leaf = std::dynamic_pointer_cast<LeafNode<T>>(node);
    if (leaf) {
      buffer.addBit(true);
      buffer.add<T>(leaf->data);
    } else {
      const InnerNodePtr inner = std::dynamic_pointer_cast<InnerNode>(node);
      buffer.addBit(false);
      writeCodebook<T>(inner->left, buffer);
      writeCodebook<T>(inner->right, buffer);
    }
  }

  template <typename T>
  NodePtr huffmanCoding(const Histogram<T>& histogram) {
    std::priority_queue<NodePtr, std::vector<NodePtr> ,NodeGreater> pq;
    
    for (const auto& element : histogram) {
      pq.push(std::make_shared<LeafNode<T>>(element.first, element.second));
    }
    
    while (pq.size() > 1) {
      const NodePtr firstElement = pq.top();
      pq.pop();
      const NodePtr secondElement = pq.top();
      pq.pop();
      const NodePtr newElement = std::make_shared<InnerNode>(firstElement,secondElement);
      pq.push(newElement);
    }
    
    return pq.top();
  }

  template <typename T>
  void treeToCodes(const NodePtr node,
                               std::deque<bool>& bits, Codes<T>& codes) {
    const LeafNodePtr<T> leaf = std::dynamic_pointer_cast<LeafNode<T>>(node);
    if (leaf) {
      std::vector<bool> bitVec;
      for (const bool b : bits) bitVec.push_back(b);
      codes[leaf->data] = bitVec;
    } else {
      const InnerNodePtr inner = std::dynamic_pointer_cast<InnerNode>(node);
      
      bits.push_back(true);
      treeToCodes(inner->left, bits, codes);
      bits.pop_back();
      
      bits.push_back(false);
      treeToCodes(inner->right, bits, codes);
      bits.pop_back();
    }
  }
  
  template <typename T>
  std::vector<char> huffmanEncode(const std::vector<T>& source) {
    if (source.empty()) return {};
    
    const auto [histogram, total] = buildHistogram<T>(source);
    const NodePtr root = huffmanCoding<T>(histogram);
    const Codes<T> codes = treeToCodes<T>(root);

    OutputBitBuffer buffer;
    writeCodebook<T>(root, buffer);
    buffer.add(total);
    encodeBuffer(source, buffer, codes);
    return buffer.getData();
  }

  template <typename T>
  NodePtr readCodebook(InputBitBuffer& buffer) {
    if (buffer.getBit()) {
      return std::make_shared<LeafNode<T>>(buffer.get<T>(), 0);
    } else {
      NodePtr left = readCodebook<T>(buffer);
      NodePtr right = readCodebook<T>(buffer);
      return std::make_shared<InnerNode>(left,right);
    }
  }
  
  template <typename T>
  T decodeElement(InputBitBuffer& buffer,
                  const NodePtr root) {
    NodePtr current = root;
    do {
      const LeafNodePtr<T> leaf = std::dynamic_pointer_cast<LeafNode<T>>(current);
      if (leaf) {
        return leaf->data;
      } else {
        const InnerNodePtr inner = std::dynamic_pointer_cast<InnerNode>(current);
        current = buffer.getBit() ? inner->left : inner->right;
      }
    } while (true);
  }
  
  template <typename T>
  std::vector<T> decodeBuffer(InputBitBuffer& buffer,
                                        const uint64_t total,
                                        const NodePtr root) {
    std::vector<T> data(total);
    for (uint64_t i = 0;i<total;++i) {
      data[i] = decodeElement<T>(buffer, root);
    }
    return data;
  }
  
  template <typename T>
  std::vector<T> huffmanDecode(const std::vector<char>& source) {
    if (source.empty()) return {};
    
    InputBitBuffer buffer(source);
    const NodePtr root = readCodebook<T>(buffer);
    const uint64_t total = buffer.get<uint64_t>();
    return decodeBuffer<T>(buffer, total, root);
  }

  template <typename T>
  void encodeBuffer(const std::vector<T>& source,
                           OutputBitBuffer& buffer,
                           const Codes<T>& codes) {
    for (const T& c : source) {
      buffer.addBits(codes.at(c));
    }
  }

  template <typename T>
  Codes<T> treeToCodes(const NodePtr node) {
    Codes<T> codes;
    std::deque<bool> bits;
    treeToCodes(node, bits, codes);
    return codes;
  }

  template <typename T>
  bool checkTree(const NodePtr node) {
    const LeafNodePtr<T> leaf = std::dynamic_pointer_cast<LeafNode<T>>(node);
    if (leaf) {
      return true;
    }

    const InnerNodePtr inner = std::dynamic_pointer_cast<InnerNode>(node);
    if (inner) {
      return checkTree<T>(inner->left) && checkTree<T>(inner->right);
    }

    return false;
  }


  template <typename T>
  void printCodes(const Codes<T>& codes) {
    for (const auto& code : codes) {
      std::cout << code.first << " : ";
      for (const bool bit : code.second) {
        std::cout << (bit ? 1 : 0);
      }
      std::cout << std::endl;
    }
  }

  template <typename T>
  void printAvgLength(const Codes<T>& codes,
                                   const Histogram<T>& histogram,
                                   uint64_t total) {
    uint64_t bitCount{0};
    for (const auto& code : codes) {
      bitCount += code.second.size() * histogram.at(code.first);
    }
    double avgLen{bitCount/double(total)};

    std::cout << "Average length " << avgLen << std::endl;
    std::cout << "Aprox. size  " << bitCount/8 << std::endl;
  }


  template <typename T>
  std::vector<std::pair<T, uint32_t>> toCanonicalEncoding1(const NodePtr root) {
    const Codes<T> codes = treeToCodes<T>(root);
    std::vector<std::pair<T, uint32_t>> codeVec;
    
    for (const auto& code : codes) {
      codeVec.push_back({code.first, code.second.size()});
    }
      
    return codeVec;
  }

  template <typename T>
  std::vector<uint32_t> toCanonicalEncoding2(const NodePtr root) {
    const std::vector<std::pair<T, uint32_t>> enc1 = toCanonicalEncoding1<T>(root);
    std::vector<uint32_t> result;

    size_t elemIndex{0};
    size_t maxIndex = 1<<(sizeof(T)*8);
    for (size_t i = 0;i<maxIndex && elemIndex < enc1.size() ;++i) {
      const T currentElem = T(i);
      if (currentElem != enc1[elemIndex].first) {
        result.push_back(0);
      } else {
        result.push_back(enc1[elemIndex++].second);
      }
    }
    
    return result;
  }

  static std::vector<bool> intToPath(uint32_t currentIndex, uint32_t length) {
    std::bitset<32> binary(currentIndex);
    std::vector<bool> path;
    for (size_t i = 0;i<length;++i) {
      path.push_back(binary[length-1-i]);
    }
    return path;
  }

  template <typename T>
  NodePtr addToTree(NodePtr node,
                    const std::vector<bool>& path,
                    const T& element,
                    const size_t current=0) {
    if (path.size() == current) {
      return std::make_shared<LeafNode<T>>(element, 0);
    } else {
      if (node == nullptr) {
        node = std::make_shared<InnerNode>();
      }
      
      const InnerNodePtr inner = std::dynamic_pointer_cast<InnerNode>(node);
      if (path[current]) {
        inner->right  = addToTree(inner->right, path, element, current+1);
      } else {
        inner->left = addToTree(inner->left, path, element, current+1);
      }

      return node;
    }
  }

  template <typename T>
  NodePtr fromCanonicalEncoding1(const std::vector<std::pair<T, uint32_t>>& encoding) {
    NodePtr root{nullptr};
    
    std::vector<std::pair<T, uint32_t>> encoding1 = encoding;
    
    std::sort(encoding1.begin(), encoding1.end(),
                     [](const std::pair<T, uint32_t>& l,
                        const std::pair<T, uint32_t>& r) {
                            return std::tie(l.second, l.first) <
                                   std::tie(r.second, r.first);
                        }
    );
    
    uint32_t currentIndex{0};
    uint32_t lastLength{0};
    for (const auto& element : encoding1) {
      if (lastLength != 0)
        currentIndex = (currentIndex + 1) << (element.second-lastLength);
      const std::vector<bool> path = intToPath(currentIndex, element.second);
      root = addToTree(root, path, element.first);
      lastLength = element.second;
    }

    return root;
  }

  template <typename T>
  NodePtr fromCanonicalEncoding2(const std::vector<uint32_t>& encoding2) {
    std::vector<std::pair<T, uint32_t>> encoding1;
    for (size_t i = 0;i<encoding2.size();++i) {
      if (encoding2[i] != 0) {
        encoding1.push_back({T(i),encoding2[i]});
      }
    }
    return fromCanonicalEncoding1(encoding1);
  }

  template <typename T>
  NodePtr canonicalize(const NodePtr root) {
    return fromCanonicalEncoding1(toCanonicalEncoding1<T>(root));
  }

  
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

    void serialize(OutputBitBuffer& buffer,
                   const Codes<uint16_t>& codeCodes,
                   const Codes<uint8_t>& posCodes) const {
      
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
    
    static bool decode(InputBitBuffer& buffer,
                       const NodePtr rootCodes,
                       const NodePtr rootPos,
                       std::vector<char>& output) {
      
      if (rootCodes == nullptr ||rootPos == nullptr) return false;
      
      const uint16_t code = decodeElement<uint16_t>(buffer, rootCodes);
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

      const uint8_t posCode = decodeElement<uint8_t>(buffer, rootPos);
      
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
  
  static size_t checkAtPos(const std::vector<char>& input, size_t pos,
                           size_t searchPos, size_t posOffset=0) {
    pos += posOffset;
    const size_t maxLength{pos-searchPos};
    size_t length = 0;
    while (pos+length < input.size() &&
           input[searchPos+length%maxLength] == input[pos+length] &&
           length < minSequenceLength+lookAheadSize-(posOffset+1)) {
      length++;
    }
    return length;
  }
  
  static void updateMap(const std::vector<char>& input, const size_t pos,
                        HashTable& startPosMap, size_t length) {
  
    if (pos+length < 3 || pos+1 >= input.size()) return;
    for (size_t i = 0;i<length;++i) {
      if (pos+i < 2) continue;
      const size_t index = (pos+i)-2;
      Tripple findTripple{input[index],input[index+1],input[index+2]};
      startPosMap.add(findTripple,index);
    }
  }

  
  static LZSSElement search(const std::vector<char>& input, const size_t pos,
                                HashTable& startPosMap) {
    LZSSElement e{0,0,input[pos]};
    if (pos+2 >= input.size()) {
      updateMap(input, pos, startPosMap, 1);
      return e;
    }
    
    Tripple searchTripple{input[pos],input[pos+1],input[pos+2]};
    
    const size_t searchStart{windowSize >= pos ? 0 : pos-(windowSize-1)};

    size_t maxLength=0;
    size_t maxPos=0;
    size_t posLength;

    if (pos > 0) {
      posLength = checkAtPos(input, pos, pos-1);
      if (posLength > maxLength || (posLength == maxLength && pos-1 > maxPos)) {
        maxLength = posLength;
        maxPos = pos-1;
      }
    }
    if (pos > 1) {
      posLength = checkAtPos(input, pos, pos-2);
      if (posLength > maxLength || (posLength == maxLength && pos-2 > maxPos)) {
        maxLength = posLength;
        maxPos = pos-2;
      }
    }


    const auto& list = startPosMap.get(searchTripple);
    for (auto iter = list.begin();
              iter != list.end();
              ++iter) {
      
      if (iter->second < searchStart) {
        startPosMap.trim(searchTripple, iter);
        break;
      }
      
      if (iter->first == searchTripple) {
        posLength = checkAtPos(input, pos, iter->second+3, 3)+3;
        if (posLength > maxLength) {
          maxLength = posLength;
          maxPos = iter->second;
        }
        if (maxLength == 258) break;
      }
    }
 
    
    if (maxLength < 3) {
      updateMap(input, pos, startPosMap, 1);
      return e;
    }
      
    updateMap(input, pos, startPosMap, maxLength);
    return {uint16_t(pos-maxPos),uint16_t(maxLength)};
  }

  static LZSSElement slowSearch(const std::vector<char>& input, size_t pos) {
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

    std::vector<char> compress(const std::vector<char>& input) {
      OutputBitBuffer buffer;
      std::vector<DeflateCode> deflateCodes;
      HashTable startPosMap(1<<14);
      
      Histogram<uint16_t> codeHistogram;
      Histogram<uint8_t> posHistogram;
      
      size_t pos{0};
      while (pos < input.size()) {
        LZSSElement element = search(input, pos, startPosMap);
 /*
        LZSSElement element_test = slowSearch(input, pos);
        if (element_test.data != element.data ||
            element_test.position != element.position ||
            element_test.length != element.length) {
          std::cout << "found difference" << std::endl;
        }
 */
                
        const DeflateCode deflateCode = element.toDeflateCode();
        addToHistogram(deflateCode.code, codeHistogram);
        
        if (element.length > 0) {
          addToHistogram(deflateCode.posCode, posHistogram);
        }
        
        deflateCodes.push_back(deflateCode);
        pos += std::max<size_t>(1,element.length);
      }
      
      addToHistogram(uint16_t(256), codeHistogram);
      
      const NodePtr codeRoot = canonicalize<uint16_t>(huffmanCoding(codeHistogram));
      const NodePtr posRoot = canonicalize<uint8_t>(huffmanCoding(posHistogram));

      
      const Codes<uint16_t> codeCodes = treeToCodes<uint16_t>(codeRoot);
      const Codes<uint8_t> posCodes = treeToCodes<uint8_t>(posRoot);
      
      const std::vector<uint32_t> codeCodebook = toCanonicalEncoding2<uint16_t>(codeRoot);
      const std::vector<uint32_t> posCodebook = toCanonicalEncoding2<uint8_t>(posRoot);
      
      auto maxElem = std::max_element(codeCodebook.begin(), codeCodebook.end());
      uint8_t bitCount = uint8_t(ceil(log2(1+*maxElem)));
      
      buffer.add(codeCodebook.size()-257, 5);
      buffer.add(bitCount, 8);
      
      for (const uint32_t sizeEntry : codeCodebook) {
        buffer.add(sizeEntry, bitCount);
      }
      
      maxElem = std::max_element(posCodebook.begin(), posCodebook.end());
      bitCount = uint8_t(ceil(log2(1+*maxElem)));
      
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
    
  void compress(const std::string& sourceFilename, const std::string& targetFilename) {
    std::ifstream sourceFile(sourceFilename, std::ios::binary);
    std::ofstream targetFile(targetFilename, std::ios::binary);
    const std::vector<char> source = std::vector<char>((std::istreambuf_iterator<char>(sourceFile)),
                                                        std::istreambuf_iterator<char>());
    
    const auto compressed = compress(source);
    targetFile.write((char*)compressed.data(), long(compressed.size()));
  }


  std::vector<char> decompress(const std::vector<char>& input) {
    std::vector<char> output;
    
    InputBitBuffer buffer(input);
    
    uint16_t codeCodebookSize = buffer.get<uint16_t>(5) + 257;
    uint8_t codeCodebookBitCount = buffer.get<uint8_t>(8);

    if (codeCodebookSize > 286) {
      throw Exception("codeCodebookSize invalid");
    }
    
    std::vector<uint32_t> codeCodebook;
    for (uint16_t i = 0;i<codeCodebookSize;++i) {
      codeCodebook.push_back(buffer.get<uint32_t>(codeCodebookBitCount));
    }
      
    uint16_t posCodebookSize = buffer.get<uint16_t>(5) + 1;
    uint8_t posCodebookBitCount = buffer.get<uint8_t>(8);

    if (posCodebookSize > 30) {
      throw Exception("codeCodebookSize invalid");
    }

    std::vector<uint32_t> posCodebook;
    for (uint16_t i = 0;i<posCodebookSize;++i) {
      posCodebook.push_back(buffer.get<uint32_t>(posCodebookBitCount));
    }

    const NodePtr rootCodes = fromCanonicalEncoding2<uint16_t>(codeCodebook);
    const NodePtr rootPos = fromCanonicalEncoding2<uint8_t>(posCodebook);
    
    if (!checkTree<uint16_t>(rootCodes)) {
      throw Exception("rootPos tree invalid");
    }
    
    if (!checkTree<uint8_t>(rootPos)) {
      throw Exception("rootPos tree invalid");
    }
    
    while (LZSSElement::decode(buffer, rootCodes, rootPos, output)) {}
      
    return output;
  }

  void decompress(const std::string& sourceFilename,
                         const std::string& targetFilename) {
    std::ifstream sourceFile(sourceFilename, std::ios::binary);
    std::ofstream targetFile(targetFilename, std::ios::binary);
    
    const std::vector<char> source = std::vector<char>((std::istreambuf_iterator<char>(sourceFile)),
                                                        std::istreambuf_iterator<char>());
      
    const std::vector<char> target = decompress(source);
    targetFile.write((char*)target.data(), long(target.size()));
  }

  
  static uint8_t paethPredictor(uint8_t left, uint8_t above, uint8_t upperLeft) {
    int32_t prediction = left + above - upperLeft;
    int32_t distanceLeft = abs(prediction-left);
    int32_t distanceAbove = abs(prediction-above);
    int32_t distanceUpperLeft = abs(prediction-upperLeft);

    if (distanceLeft <= distanceAbove &&
        distanceLeft <= distanceUpperLeft)
      return left;
    else
      if (distanceAbove <= distanceUpperLeft)
        return above;
      else
        return upperLeft;
  }

  
  static size_t imagePos(uint32_t width, uint32_t height,
                         uint32_t x, uint32_t y, uint32_t c) {
    return x+y*width+c*(width*height);
  }

  static Image loadJhk(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    
    uint32_t width, height, componentCount;
    file.read((char*)&width, sizeof(width));
    file.read((char*)&height, sizeof(height));
    file.read((char*)&componentCount, sizeof(componentCount));
    
    const std::vector<char> source = std::vector<char>((std::istreambuf_iterator<char>(file)),
                                                        std::istreambuf_iterator<char>());
    std::vector<char> data = decompress(source);
    std::vector<uint8_t> uintData(width*height*componentCount);

    for (uint32_t x = 0;x<width;++x) {
      for (uint32_t c = 0;c<componentCount;++c) {
        uintData[c+x*componentCount] = uint8_t(data[imagePos(width,height,x,0,c)]);
      }
    }

    for (uint32_t y = 0;y<height;++y) {
      for (uint32_t c = 0;c<componentCount;++c) {
        uintData[c+y*width*componentCount] = uint8_t(data[imagePos(width,height,0,y,c)]);
      }
    }

    for (uint32_t y = 1;y<height;++y) {
      for (uint32_t x = 1;x<width;++x) {
        for (uint32_t c = 0;c<componentCount;++c) {
          const uint8_t difference = uint8_t(data[imagePos(width,height,x,y,c)]);
          const uint8_t predicted = paethPredictor(uintData[c+(x-1+y*width)*componentCount],
                                                   uintData[c+(x+(y-1)*width)*componentCount],
                                                   uintData[c+(x-1+(y-1)*width)*componentCount]);
          uintData[c+componentCount*(x+y*width)] = difference+predicted;
        }
      }
    }

    return {width,height,componentCount,uintData};
  }

  static void saveJhk(const std::string& filename, const Image& image) {
  
    std::vector<char> charData(image.data.size());
    
     for (uint32_t x = 0;x<image.width;++x) {
      for (uint32_t c = 0;c<image.componentCount;++c) {
        charData[imagePos(image.width,image.height,x,0,c)] = char(image.getValue(x, 0, c));
      }
    }
    
    for (uint32_t y = 0;y<image.height;++y) {
      for (uint32_t c = 0;c<image.componentCount;++c) {
        charData[imagePos(image.width,image.height,0,y,c)] = char(image.getValue(0, y, c));
      }
    }
    
    for (uint32_t y = 1;y<image.height;++y) {
      for (uint32_t x = 1;x<image.width;++x) {
        for (uint32_t c = 0;c<image.componentCount;++c) {
          const uint8_t realValue = image.getValue(x, y, c);
          const uint8_t predicted = paethPredictor(image.getValue(x-1, y, c),
                                                   image.getValue(x, y-1, c),
                                                   image.getValue(x-1, y-1, c));
          charData[imagePos(image.width,image.height,x,y,c)] = char(realValue-predicted);
        }
      }
    }
    
    /*
    Image image2{image};
    for (size_t i = 0;i<image.data.size();++i)
      image2.data[i] = uint8_t(charData[i]);
    BMP::save("test.bmp", image2);
    */
    
    std::ofstream file(filename, std::ios::binary);
    file.write((char*)&image.width, sizeof(image.width));
    file.write((char*)&image.height, sizeof(image.height));
    file.write((char*)&image.componentCount, sizeof(image.componentCount));
    
    std::vector<char> compressedData = compress(charData);
    file.write((char*)compressedData.data(), long(compressedData.size()));
  }

  void bmp2jhk(const std::string& sourceFilename,
               const std::string& targetFilename) {
    saveJhk(targetFilename, BMP::load(sourceFilename));
  }

  void jhk2bmp(const std::string& sourceFilename,
               const std::string& targetFilename) {
    BMP::save(targetFilename, loadJhk(sourceFilename));
  }

};
