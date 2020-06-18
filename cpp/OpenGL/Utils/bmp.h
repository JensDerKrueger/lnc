#include <fstream>
#include <iostream>
#include <exception>
#include <string>
#include <sstream>

#include "Vec2i.h"

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

    struct Image {
        uint32_t width;
        uint32_t height;
        uint32_t componentCount;
        std::vector<uint8_t> data;
        
        size_t computeIndex(uint32_t x, uint32_t y, uint32_t component) const {
            return component+(x+y*width)*componentCount;
        }
        
        uint8_t getValue(uint32_t x, uint32_t y, uint32_t component) const {
            return data[computeIndex(x, y, component)];
        }

        void setValue(uint32_t x, uint32_t y, uint32_t component, uint8_t value) {
            data[computeIndex(x, y, component)] = value;
        }

    };


	bool save(const std::string& filename, uint32_t w, uint32_t h,
              const std::vector<uint8_t>& data, uint8_t iComponentCount = 3);


    Image load(const std::string& filename);

    void blit(const Image& source, const Vec2ui& sourceStart, const Vec2ui& sourceEnd,
              Image& target, const Vec2ui& targetStart, bool skipChecks=false);
}
