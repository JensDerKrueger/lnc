#pragma once

#include <iostream>
#include <fstream>
#include <queue>
#include <map>


namespace Compression {

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
  void buildCodes(const NodePtr node, std::deque<bool>& bits, Codes<T>& codes);

  template <typename T>
  Codes<T> buildCodes(const NodePtr node);

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
  std::vector<T> decodeBuffer(InputBitBuffer& buffer, const uint64_t total,
                              const NodePtr root);

  template <typename T>
  void writeCodebook(const NodePtr node, OutputBitBuffer& buffer);

  void huffmanEncode(const std::string& sourceFilename,
                     const std::string& targetFilename);
  void huffmanDecode(const std::string& sourceFilename, const
                     std::string& targetFilename);

};


template <typename T>
std::vector<char> Compression::huffmanEncode(const std::vector<T>& source) {
  if (source.empty()) return {};
  
  const auto [histogram, total] = buildHistogram<T>(source);
  const NodePtr root = huffmanCoding<T>(histogram);
  const Codes<T> codes = buildCodes<T>(root);

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
std::vector<T> Compression::decodeBuffer(InputBitBuffer& buffer,
                                      const uint64_t total,
                                      const NodePtr root) {
  std::vector<T> data(total);
  
  for (uint64_t i = 0;i<total;++i) {
    NodePtr current = root;
    do {
      const LeafNodePtr<T> leaf = std::dynamic_pointer_cast<LeafNode<T>>(current);
      if (leaf) {
        data[i] = leaf->data;
        break;
      } else {
        const InnerNodePtr inner = std::dynamic_pointer_cast<InnerNode>(current);
        current = buffer.getBit() ? inner->left : inner->right;
      }
    } while (true);
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
void Compression::buildCodes(const Compression::NodePtr node,
                             std::deque<bool>& bits, Codes<T>& codes) {
  const LeafNodePtr<T> leaf = std::dynamic_pointer_cast<LeafNode<T>>(node);
  if (leaf) {
    std::vector<bool> bitVec;
    for (const bool b : bits) bitVec.push_back(b);
    codes[leaf->data] = bitVec;
  } else {
    const InnerNodePtr inner = std::dynamic_pointer_cast<InnerNode>(node);
    
    bits.push_back(true);
    buildCodes(inner->left, bits, codes);
    bits.pop_back();
    
    bits.push_back(false);
    buildCodes(inner->right, bits, codes);
    bits.pop_back();
  }
}

template <typename T>
Compression::Codes<T> Compression::buildCodes(const NodePtr node) {
  Codes<T> codes;
  std::deque<bool> bits;
  buildCodes(node, bits, codes);
  return codes;
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
