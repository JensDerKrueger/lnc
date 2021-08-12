#pragma once

#include <exception>
#include <string>
#include <vector>

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
  
  void bmp2jhk(const std::string& sourceFilename,
               const std::string& targetFilename);

  void jhk2bmp(const std::string& sourceFilename,
               const std::string& targetFilename);

};
