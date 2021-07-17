#include <iostream>
#include <fstream>
#include <queue>
#include <map>

#include "Compression.h"

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

static std::pair<Histogram, uint64_t> genHistogram(const std::vector<char>& source) {
  Histogram histogram;
  uint64_t total{0};
  for (const char c : source) {
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

static void writeCodebook(const NodePtr node, Compression::OutputBitBuffer& buffer) {
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

static void encodeBuffer(const std::vector<char>& source, Compression::OutputBitBuffer& buffer, const Codes& codes) {
  for (const char c : source) {
    buffer.addBits(codes.at(c));
  }
}

static NodePtr readCodebook(Compression::InputBitBuffer& buffer) {
  if (buffer.getBit()) {
    return std::make_shared<LeafNode>(buffer.getChar(), 0);
  } else {
    NodePtr left = readCodebook(buffer);
    NodePtr right = readCodebook(buffer);
    return std::make_shared<InnerNode>(left,right);
  }
}

static std::vector<char> decodeBuffer(Compression::InputBitBuffer& buffer, const uint64_t total, const NodePtr root) {
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
  
  return data;
}

void Compression::huffmanDecode(const std::string& sourceFilename, const std::string& targetFilename) {
  std::ifstream sourceFile(sourceFilename, std::ios::binary);
  std::ofstream targetFile(targetFilename, std::ios::binary);
  const std::vector<char> source = std::vector<char>((std::istreambuf_iterator<char>(sourceFile)),
                                                       std::istreambuf_iterator<char>());
  std::vector<char> target = huffmanDecode(source);
  targetFile.write((char*)target.data(), long(target.size()));
}


void Compression::huffmanEncode(const std::string& sourceFilename, const std::string& targetFilename) {
  std::ifstream sourceFile(sourceFilename, std::ios::binary);
  std::ofstream targetFile(targetFilename, std::ios::binary);
  const std::vector<char> source = std::vector<char>((std::istreambuf_iterator<char>(sourceFile)),
                                                           std::istreambuf_iterator<char>());
  const std::vector<char> target = huffmanEncode(source);
  targetFile.write((char*)target.data(), long(target.size()));
}

std::vector<char> Compression::huffmanEncode(const std::vector<char>& source) {
  if (source.empty()) return {};
  
  const auto [histogram, total] = genHistogram(source);
  const NodePtr root = huffmanCoding(histogram);
  const Codes codes = buildCodes(root);

  OutputBitBuffer buffer;
  writeCodebook(root, buffer);
  buffer.add(total);
  encodeBuffer(source, buffer, codes);
  return buffer.getData();
}

std::vector<char> Compression::huffmanDecode(const std::vector<char>& source) {
  if (source.empty()) return {};
  
  InputBitBuffer buffer(source);
  const NodePtr root = readCodebook(buffer);
  const uint64_t total = buffer.get<uint64_t>();
  return decodeBuffer(buffer, total, root);
}
