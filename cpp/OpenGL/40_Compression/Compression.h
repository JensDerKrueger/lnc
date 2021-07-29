#pragma once

#include <iostream>
#include <fstream>
#include <queue>
#include <map>
#include <bitset>
#include <exception>
#include <memory>
#include <algorithm>
#include <math.h>

namespace Compression {

  class Exception : public std::exception {
    public:
      Exception(const std::string& whatStr) : whatStr(whatStr) {}
      virtual const char* what() const throw() {
        return whatStr.c_str();
      }
    private:
      std::string whatStr;
  };

  
  template <typename T>
  using Histogram = std::map<T, size_t>;
  
  template <typename T>
  using Codes = std::map<T, std::vector<bool>>;

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
  void encodeBuffer(const std::vector<T>& source, OutputBitBuffer& buffer,
                    const Codes<T>& codes);

  template <typename T>
  void addToHistogram(const T& e, Histogram<T>& histogram);

  template <typename T>
  std::pair<Histogram<T>, uint64_t> buildHistogram(const std::vector<T>& source);

  template <typename T>
  NodePtr huffmanCoding(const Histogram<T>& histogram) ;

  template <typename T>
  void treeToCodes(const NodePtr node, std::deque<bool>& bits, Codes<T>& codes);

  template <typename T>
  bool checkTree(const NodePtr node);

  template <typename T>
  NodePtr canonicalize(const NodePtr root);
  
  template <typename T>
  std::vector<std::pair<T, uint32_t>> toCanonicalEncoding1(const NodePtr root);

  template <typename T>
  std::vector<uint32_t> toCanonicalEncoding2(const NodePtr root);

  template <typename T>
  NodePtr fromCanonicalEncoding1(const std::vector<std::pair<T, uint32_t>>& encoding);

  template <typename T>
  NodePtr fromCanonicalEncoding2(const std::vector<uint32_t>& encoding);

  template <typename T>
  NodePtr addToTree(NodePtr root, const std::vector<bool>& path,
                    const T& element, const size_t current=0);

  std::vector<bool> intToPath(uint32_t currentIndex, uint32_t length);

  uint32_t pathToInt(std::vector<bool> path);
  
  template <typename T>
  Codes<T> treeToCodes(const NodePtr root);

  template <typename T>
  void printCodes(const Codes<T>& codes);

  template <typename T>
  void printAvgLength(const Codes<T>& codes, const Histogram<T>& histogram,
                      uint64_t total);

  template <typename T>
  std::vector<char> huffmanEncode(const std::vector<T>& source);
  
  template <typename T>
  std::vector<T> huffmanDecode(const std::vector<char>& source);

  template <typename T>
  NodePtr readCodebook(InputBitBuffer& buffer);

  template <typename T>
  T decodeElement(InputBitBuffer& buffer, const NodePtr root);

  template <typename T>
  std::vector<T> decodeBuffer(InputBitBuffer& buffer, const uint64_t total,
                              const NodePtr root);

  template <typename T>
  void writeCodebook(const NodePtr node, OutputBitBuffer& buffer);

  void huffmanEncode(const std::string& sourceFilename,
                     const std::string& targetFilename);
  void huffmanDecode(const std::string& sourceFilename, const
                     std::string& targetFilename);

  void decompress(const std::string& sourceFilename,
                  const std::string& targetFilename);

  void compress(const std::string& sourceFilename,
                const std::string& targetFilename);
  
  std::vector<char> decompress(const std::vector<char>& input);
  std::vector<char> compress(const std::vector<char>& input);
};


template <typename T>
std::vector<char> Compression::huffmanEncode(const std::vector<T>& source) {
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
std::vector<T> Compression::huffmanDecode(const std::vector<char>& source) {
  if (source.empty()) return {};
  
  InputBitBuffer buffer(source);
  const NodePtr root = readCodebook<T>(buffer);
  const uint64_t total = buffer.get<uint64_t>();
  return decodeBuffer<T>(buffer, total, root);
}

template <typename T>
Compression::NodePtr Compression::readCodebook(InputBitBuffer& buffer) {
  if (buffer.getBit()) {
    return std::make_shared<LeafNode<T>>(buffer.get<T>(), 0);
  } else {
    NodePtr left = readCodebook<T>(buffer);
    NodePtr right = readCodebook<T>(buffer);
    return std::make_shared<InnerNode>(left,right);
  }
}

template <typename T>
T Compression::decodeElement(InputBitBuffer& buffer,
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
std::vector<T> Compression::decodeBuffer(InputBitBuffer& buffer,
                                      const uint64_t total,
                                      const NodePtr root) {
  std::vector<T> data(total);
  for (uint64_t i = 0;i<total;++i) {
    data[i] = decodeElement<T>(buffer, root);
  }
  return data;
}

template <typename T>
void Compression::writeCodebook(const NodePtr node, OutputBitBuffer& buffer) {
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
void Compression::encodeBuffer(const std::vector<T>& source,
                         OutputBitBuffer& buffer,
                         const Codes<T>& codes) {
  for (const T& c : source) {
    buffer.addBits(codes.at(c));
  }
}

template <typename T>
void Compression::addToHistogram(const T& e, Histogram<T>& histogram) {
  if (histogram.find(e) == histogram.end()) {
    histogram[e] = 1;
  } else {
    histogram[e]++;
  }
}


template <typename T>
std::pair<Compression::Histogram<T>, uint64_t> Compression::buildHistogram(const std::vector<T>& source) {
  Histogram<T> histogram;
  uint64_t total{0};
  for (const T& e : source) {
    addToHistogram<T>(e,histogram);
    ++total;
  }
  return {histogram, total};
}

template <typename T>
Compression::NodePtr Compression::huffmanCoding(const Histogram<T>& histogram) {
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
void Compression::treeToCodes(const Compression::NodePtr node,
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
Compression::Codes<T> Compression::treeToCodes(const NodePtr node) {
  Codes<T> codes;
  std::deque<bool> bits;
  treeToCodes(node, bits, codes);
  return codes;
}

template <typename T>
bool Compression::checkTree(const NodePtr node) {
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
void Compression::printCodes(const Codes<T>& codes) {
  for (const auto& code : codes) {
    std::cout << code.first << " : ";
    for (const bool bit : code.second) {
      std::cout << (bit ? 1 : 0);
    }
    std::cout << std::endl;
  }
}

template <typename T>
void Compression::printAvgLength(const Codes<T>& codes,
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
std::vector<std::pair<T, uint32_t>> Compression::toCanonicalEncoding1(const NodePtr root) {
  const Codes<T> codes = treeToCodes<T>(root);
  std::vector<std::pair<T, uint32_t>> codeVec;
  
  for (const auto& code : codes) {
    codeVec.push_back({code.first, code.second.size()});
  }
    
  return codeVec;
}

template <typename T>
std::vector<uint32_t> Compression::toCanonicalEncoding2(const NodePtr root) {
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

template <typename T>
Compression::NodePtr Compression::addToTree(Compression::NodePtr node,
                                            const std::vector<bool>& path,
                                            const T& element,
                                            const size_t current) {
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
Compression::NodePtr Compression::fromCanonicalEncoding1(const std::vector<std::pair<T, uint32_t>>& encoding) {
  Compression::NodePtr root{nullptr};
  
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
Compression::NodePtr Compression::fromCanonicalEncoding2(const std::vector<uint32_t>& encoding2) {  
  std::vector<std::pair<T, uint32_t>> encoding1;
  for (size_t i = 0;i<encoding2.size();++i) {
    if (encoding2[i] != 0) {
      encoding1.push_back({T(i),encoding2[i]});
    }
  }
  return fromCanonicalEncoding1(encoding1);
}

template <typename T>
Compression::NodePtr Compression::canonicalize(const Compression::NodePtr root) {
  return fromCanonicalEncoding1(toCanonicalEncoding1<T>(root));
}


