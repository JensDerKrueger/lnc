#pragma once

#include <fstream>
#include <iostream>
#include <exception>
#include <string>
#include <sstream>

#include "Vec2.h"
#include "Image.h"

namespace BMP {
  class BMPException : public std::exception {
    public:
      BMPException(const std::string& whatStr) : whatStr(whatStr) {}
      virtual const char* what() const throw() {
        return whatStr.c_str();
      }
    private:
      std::string whatStr;
  };

  bool save(const std::string& filename, const Image& source);


  bool save(const std::string& filename, uint32_t w, uint32_t h,
            const std::vector<uint8_t>& data, uint8_t iComponentCount = 3);

  bool save(const std::string& filename, uint32_t w, uint32_t h,
            const std::vector<float>& data, uint8_t iComponentCount = 3);


  Image load(const std::string& filename);

  void blit(const Image& source, const Vec2ui& sourceStart, const Vec2ui& sourceEnd,
            Image& target, const Vec2ui& targetStart, bool skipChecks=false);
}
