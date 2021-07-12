#include <iostream>
#include <fstream>
#include <queue>
#include <map>

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

static Histogram genHistogram(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  Histogram histogram;
  char c;
  while (file.get(c)) {
    if (histogram.find(c) == histogram.end()) {
      histogram[c] = 1;
    } else {
      histogram[c]++;
    }
  }
  return histogram;
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

int main(int argc, char** argv) {
  Histogram histogram = genHistogram("main.cpp");
  NodePtr root = huffmanCoding(histogram);
  Codes codes = buildCodes(root);
  printCodes(codes);
  return EXIT_SUCCESS;
}
