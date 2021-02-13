#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>

#include <FontRenderer.h>

int main(int argc, char ** argv) {
  if (argc != 5) {
    std::cout << "Usage " << argv[0] << " font.bmp font.pos target.h varname" << std::endl;
    return EXIT_FAILURE;
  }
  
  try {
    Image fontImage = BMP::load(argv[1]);
    if (fontImage.componentCount == 3)
      fontImage.generateAlphaFromLuminance();
    
    FontRenderer r{fontImage,argv[2]};
    
    std::ofstream outStream(argv[3]);
    if (!outStream.is_open())  {
      std::cout << "Can't open output file " << argv[3] << std::endl;
      return EXIT_FAILURE;
    }
    
    outStream << r.toCode(argv[4]);

    outStream.close();
  } catch (const BMP::BMPException& e) {
    std::cout << "Can't load input file " << argv[1] << " (Error:" << e.what() << ")" << std::endl;
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
