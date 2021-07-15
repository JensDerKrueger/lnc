#include <iostream>
#include <fstream>
#include <queue>
#include <map>

class BitBuffer {
protected:
  std::vector<uint8_t> data;
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
      data.push_back(uint8_t(c));
    } else {
      const uint8_t msb = uint8_t(c) >> bitInLastByte;
      const uint8_t lsb = uint8_t(uint8_t(c) << (8-bitInLastByte));
      data.back() = data.back() | msb;
      data.push_back(lsb);
    }
  }
  
  template <typename T>
  void add(const T& data) {
    char* p = (char*)&data;
    for (size_t i = 0;i<sizeof(T);++i) {
      addChar(p[i]);
    }
  }
  
  void writeToFile(std::ofstream& file) {
    file.write((char*)data.data(), long(data.size()));
  }
  
private:
  uint8_t bitInLastByte{0};
  
};


class InputBitBuffer : public BitBuffer {
public:
  
  InputBitBuffer(std::ifstream& file) {
    data = std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
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
      const uint8_t lsb = data[currentByte] >> (8-bitInCurrentByte);
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

  
private:
  uint8_t bitInCurrentByte{0};
  uint64_t currentByte{0};
  
};

using Histogram = std::map<char, size_t>;
using Codes = std::map<char, std::vector<bool>>;

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

struct LeafNode : Node {
  char data;
  LeafNode(char data, size_t frequency) :
  Node{frequency},
  data{data}
  {}
  
  virtual ~LeafNode() {}
};
using LeafNodePtr = std::shared_ptr<LeafNode>;


struct NodeGreater {
  bool operator() (NodePtr left, NodePtr right) const {
    return left->frequency > right->frequency;
  }
};

static std::pair<Histogram, uint64_t> genHistogram(std::ifstream& file) {
  Histogram histogram;
  uint64_t total{0};
  char c;
  while (file.get(c)) {
    if (histogram.find(c) == histogram.end()) {
      histogram[c] = 1;
    } else {
      histogram[c]++;
    }
    ++total;
  }
  return {histogram, total};
}

static NodePtr huffmanCoding(const Histogram& histogram) {
  std::priority_queue<NodePtr, std::vector<NodePtr> ,NodeGreater> pq;
  
  for (const auto& element : histogram) {
    pq.push(std::make_shared<LeafNode>(element.first, element.second));
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


static void buildCodes(const NodePtr node, std::deque<bool>& bits, Codes& codes) {
  const LeafNodePtr leaf = std::dynamic_pointer_cast<LeafNode>(node);
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

static Codes buildCodes(const NodePtr node) {
  Codes codes;
  std::deque<bool> bits;
  buildCodes(node, bits, codes);
  return codes;
}

static void printCodes(const Codes& codes) {
  for (const auto& code : codes) {
    std::cout << code.first << " : ";
    for (const bool bit : code.second) {
      std::cout << (bit ? 1 : 0);
    }
    std::cout << std::endl;
  }
}

static void printAvgLength(const Codes& codes, const Histogram& histogram, uint64_t total) {
  uint64_t bitCount{0};
  for (const auto& code : codes) {
    bitCount += code.second.size() * histogram.at(code.first);
  }
  double avgLen{bitCount/double(total)};

  std::cout << "Average length " << avgLen << std::endl;
  std::cout << "Aprox. size  " << bitCount/8 << std::endl;
}

static void writeCodebook(const NodePtr node, OutputBitBuffer& buffer) {
  const LeafNodePtr leaf = std::dynamic_pointer_cast<LeafNode>(node);
  if (leaf) {
    buffer.addBit(true);
    buffer.addChar(leaf->data);
  } else {
    const InnerNodePtr inner = std::dynamic_pointer_cast<InnerNode>(node);
    buffer.addBit(false);
    writeCodebook(inner->left, buffer);
    writeCodebook(inner->right, buffer);
  }
}

static void encodeFile(std::ifstream& sourceFile, OutputBitBuffer& buffer, const Codes& codes) {
  sourceFile.clear();
  sourceFile.seekg(0, std::ios::beg);
  
  char c;
  while (sourceFile.get(c)) {
    buffer.addBits(codes.at(c));
  }
}

static void encode(const std::string& sourceFilename, const std::string& targetFilename) {
  std::ifstream sourceFile(sourceFilename, std::ios::binary);
  std::ofstream targetFile(targetFilename, std::ios::binary);
  
  const auto [histogram, total] = genHistogram(sourceFile);
  const NodePtr root = huffmanCoding(histogram);
  const Codes codes = buildCodes(root);

  OutputBitBuffer buffer;
  writeCodebook(root, buffer);
  buffer.add(total);
  encodeFile(sourceFile, buffer, codes);
  buffer.writeToFile(targetFile);
}

static NodePtr readCodebook(InputBitBuffer& buffer) {
  if (buffer.getBit()) {
    return std::make_shared<LeafNode>(buffer.getChar(), 0);
  } else {
    NodePtr left = readCodebook(buffer);
    NodePtr right = readCodebook(buffer);
    return std::make_shared<InnerNode>(left,right);
  }
}

static void decodeFile(InputBitBuffer& buffer, const uint64_t total, const NodePtr root, std::ofstream& targetFile) {
  std::vector<char> data(total);
  
  for (uint64_t i = 0;i<total;++i) {
    NodePtr current = root;
    do {
      const LeafNodePtr leaf = std::dynamic_pointer_cast<LeafNode>(current);
      if (leaf) {
        data[i] = leaf->data;
        break;
      } else {
        const InnerNodePtr inner = std::dynamic_pointer_cast<InnerNode>(current);
        current = buffer.getBit() ? inner->left : inner->right;
      }
    } while (true);
  }
  targetFile.write((char*)data.data(),long(data.size()));
}

static void decode(const std::string& sourceFilename, const std::string& targetFilename) {
  std::ifstream sourceFile(sourceFilename, std::ios::binary);
  std::ofstream targetFile(targetFilename, std::ios::binary);

  InputBitBuffer buffer(sourceFile);
  const NodePtr root = readCodebook(buffer);
  const uint64_t total = buffer.get<uint64_t>();
  decodeFile(buffer, total, root, targetFile);
}


int main(int argc, char** argv) {
  encode("main.cpp", "main.enc");
  decode("main.enc", "main2.cpp");
  return EXIT_SUCCESS;
}
