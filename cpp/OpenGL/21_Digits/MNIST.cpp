#include <climits>
#include <fstream>
#include <iostream>

#include "MNIST.h"

template <typename T> T swap_endian(T u) {
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");
    union {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;
    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];
    return dest.u;
}


MNIST::MNIST(const std::string& imageFilename, const std::string& labelFilename) {
  int32_t i;

  std::ifstream imageFile{imageFilename, std::ifstream::binary };
  if (!imageFile) throw MNISTFileException{std::string("Unable to read file ")+imageFilename};

  imageFile.read((char*)&i, sizeof(i));
  if (swap_endian(i) != 2051) throw MNISTFileException{std::string("Invalid magic in file ")+imageFilename};

  imageFile.read((char*)&i, sizeof(i));
  size_t itemCount = size_t(swap_endian(i));
  data.resize(itemCount);

  imageFile.read((char*)&i, sizeof(i));
  if (swap_endian(i) != 28) throw MNISTFileException{std::string("Invalid image rows in file ")+imageFilename};
  imageFile.read((char*)&i, sizeof(i));
  if (swap_endian(i) != 28) throw MNISTFileException{std::string("Invalid image columns in file ")+imageFilename};

  for (size_t j = 0;j<itemCount;++j) {
    data[j].image.resize(28*28);
    imageFile.read((char*)data[j].image.data(), 28*28);
  }

  imageFile.close();

  std::ifstream labelFile{labelFilename, std::ifstream::binary };
  if (!labelFile) throw MNISTFileException{std::string("Unable to read file ")+labelFilename};
  
  labelFile.read((char*)&i, sizeof(i));
  if (swap_endian(i) != 2049) throw MNISTFileException{std::string("Invalid magic in file ")+labelFilename};
  
  labelFile.read((char*)&i, sizeof(i));
  if( size_t(swap_endian(i)) != itemCount)
    throw MNISTFileException{std::string("Invalid item count in file ")+labelFilename};

  uint8_t c;
  for (size_t j = 0;j<itemCount;++j) {
    labelFile.read((char*)&c, sizeof(c));
    data[j].label = c;
  }
  
  labelFile.close();
}
