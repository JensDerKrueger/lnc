#include <fstream>
#include <iostream>
#include <exception>
#include <string>
#include <sstream>

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
    };


	bool save(const std::string& filename, uint32_t w, uint32_t h,
              const std::vector<uint8_t>& data);

    Image load(const std::string& filename);
}
