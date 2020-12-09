#pragma once

#include <string>
#include <vector>
#include <exception>

#include <LA.h>

struct MNISTElement {
  std::vector<uint8_t> image;
  uint8_t label;
};

class MNISTFileException : std::exception {
public:
  MNISTFileException(const std::string& desc) : desc(desc) {}
  const char* what() const noexcept {return desc.c_str();}
private:
  std::string desc;
};


class MNIST {
public:
  MNIST(const std::string& imageFilename, const std::string& labelFilename);

  std::vector<MNISTElement> data;
};
