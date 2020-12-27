#pragma once

#include "Volume.h"

class QVisFileException : std::exception {
public:
  QVisFileException(const std::string& desc) : desc(desc) {}
  const char* what() const noexcept {return desc.c_str();}
private:
  std::string desc;
};


class QVisDatLine {
public:
  QVisDatLine(const std::string input);
  
  std::string id;
  std::string value;
  
private:
  void ltrim(std::string &s);
  void rtrim(std::string &s);
  void trim(std::string &s);
};

class QVis {
public:
  QVis(const std::string& filename);
  void load(const std::string& filename);
  
  Volume volume;
  
private:
  std::vector<std::string> tokenize(const std::string& str) const;
};
