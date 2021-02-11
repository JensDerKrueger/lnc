#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>

#include <Image.h>
#include <bmp.h>

int main(int argc, char ** argv) {
  if (argc != 3) {
    std::cout << "Usage " << argv[0] << " source.bmp target.cpp" << std::endl;
    return EXIT_FAILURE;
  }
  
  try {
    const Image image = BMP::load(argv[1]);
    
    std::ofstream outStream(argv[2]);
    if (!outStream.is_open())  {
      std::cout << "Can't open output file " << argv[2] << std::endl;
      return EXIT_FAILURE;
    }
    
    outStream << image.toCode();

    outStream.close();
  } catch (const BMP::BMPException& e) {
    std::cout << "Can't load input file " << argv[1] << " (Error:" << e.what() << ")" << std::endl;
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
