#include <iostream>

#include "Compression.h"

#include <bmp.h>


#include <fstream>

static uint8_t paethPredictor(uint8_t left, uint8_t above, uint8_t upperLeft) {
  int32_t prediction = left + above - upperLeft;
  int32_t distanceLeft = abs(prediction-left);
  int32_t distanceAbove = abs(prediction-above);
  int32_t distanceUpperLeft = abs(prediction-upperLeft);

  if (distanceLeft <= distanceAbove &&
      distanceLeft <= distanceUpperLeft)
    return left;
  else
    if (distanceAbove <= distanceUpperLeft)
      return above;
    else
      return upperLeft;
}

static size_t pos(uint32_t width, uint32_t height,
           uint32_t x, uint32_t y, uint32_t c) {
  return x+y*width+c*(width*height);
}

static Image loadJhk(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  
  uint32_t width, height, componentCount;
  file.read((char*)&width, sizeof(width));
  file.read((char*)&height, sizeof(height));
  file.read((char*)&componentCount, sizeof(componentCount));
  
  const std::vector<char> source = std::vector<char>((std::istreambuf_iterator<char>(file)),
                                                      std::istreambuf_iterator<char>());
  std::vector<char> data = Compression::decompress(source);
  std::vector<uint8_t> uintData(width*height*componentCount);

  for (uint32_t x = 0;x<width;++x) {
    for (uint32_t c = 0;c<componentCount;++c) {
      uintData[c+x*componentCount] = uint8_t(data[pos(width,height,x,0,c)]);
    }
  }

  for (uint32_t y = 0;y<height;++y) {
    for (uint32_t c = 0;c<componentCount;++c) {
      uintData[c+y*width*componentCount] = uint8_t(data[pos(width,height,0,y,c)]);
    }
  }

  for (uint32_t y = 1;y<height;++y) {
    for (uint32_t x = 1;x<width;++x) {
      for (uint32_t c = 0;c<componentCount;++c) {
        const uint8_t difference = uint8_t(data[pos(width,height,x,y,c)]);
        const uint8_t predicted = paethPredictor(uintData[c+(x-1+y*width)*componentCount],
                                                 uintData[c+(x+(y-1)*width)*componentCount],
                                                 uintData[c+(x-1+(y-1)*width)*componentCount]);
        uintData[c+componentCount*(x+y*width)] = difference+predicted;
      }
    }
  }

  return {width,height,componentCount,uintData};
}

static void saveJhk(const std::string& filename, const Image& image) {
  
  std::vector<char> charData(image.data.size());
  for (uint32_t x = 0;x<image.width;++x) {
    for (uint32_t c = 0;c<image.componentCount;++c) {
      charData[pos(image.width,image.height,x,0,c)] = char(image.getValue(x, 0, c));
    }
  }
  
  for (uint32_t y = 0;y<image.height;++y) {
    for (uint32_t c = 0;c<image.componentCount;++c) {
      charData[pos(image.width,image.height,0,y,c)] = char(image.getValue(0, y, c));
    }
  }


  for (uint32_t y = 1;y<image.height;++y) {
    for (uint32_t x = 1;x<image.width;++x) {
      for (uint32_t c = 0;c<image.componentCount;++c) {
        const uint8_t realValue = image.getValue(x, y, c);
        const uint8_t predicted = paethPredictor(image.getValue(x-1, y, c),
                                                 image.getValue(x, y-1, c),
                                                 image.getValue(x-1, y-1, c));
        charData[x+y*image.width+c*(image.width*image.height)] = char(realValue-predicted);
      }
    }
  }
  
  std::ofstream file(filename, std::ios::binary);
  file.write((char*)&image.width, sizeof(image.width));
  file.write((char*)&image.height, sizeof(image.height));
  file.write((char*)&image.componentCount, sizeof(image.componentCount));
  std::vector<char> compressedData = Compression::compress(charData);
  file.write((char*)compressedData.data(), long(compressedData.size()));
}

static void bmp2jhk(const std::string& sourceFilename,
                    const std::string& targetFilename) {
  saveJhk(targetFilename, BMP::load(sourceFilename));
}

static void jhk2bmp(const std::string& sourceFilename,
                    const std::string& targetFilename) {
  BMP::save(targetFilename, loadJhk(sourceFilename));
}

static void showUsage(char** argv) {
  std::cout << "Usage:" << std::endl;
  std::cout << "\t bmp to jhk " << argv[0] <<
               " c input output" << std::endl;
  std::cout << "\t jhk to bmp " << argv[0] <<
               " u input output" << std::endl;
}

int main(int argc, char** argv) {
  
  
  
  if (argc != 4) {
    showUsage(argv);
    return EXIT_FAILURE;
  }
  
  if (std::string{argv[1]} == "c")
    bmp2jhk(argv[2], argv[3]);
  else if (std::string{argv[1]} == "u")
    jhk2bmp(argv[2], argv[3]);
  else {
    showUsage(argv);
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}

/*
 
#include <iostream>

#include "Compression.h"

static void showUsage(char** argv) {
  std::cout << "Usage:" << std::endl;
  std::cout << "\t compress a file " << argv[0] <<
               " c input output" << std::endl;
  std::cout << "\t uncompress a file " << argv[0] <<
               " u input output" << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 4) {
    showUsage(argv);
    return EXIT_FAILURE;
  }
  
  if (std::string{argv[1]} == "c")
    Compression::compress(argv[2], argv[3]);
  else if (std::string{argv[1]} == "u")
    Compression::decompress(argv[2], argv[3]);
  else {
    showUsage(argv);
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}

*/
