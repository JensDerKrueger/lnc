#include <iostream>
#include "Compression.h"
 
static void showUsage(char** argv) {
  std::cout << "Usage:" << std::endl;
  std::cout << "\t compress a file " << argv[0] <<
               " c input output" << std::endl;
  std::cout << "\t uncompress a file " << argv[0] <<
               " u input output" << std::endl;
  std::cout << "\t bmp to jhk " << argv[0] <<
               " e input output" << std::endl;
  std::cout << "\t jhk to bmp " << argv[0] <<
               " d input output" << std::endl;

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
  else if (std::string{argv[1]} == "e")
    Compression::bmp2jhk(argv[2], argv[3]);
  else if (std::string{argv[1]} == "d")
    Compression::jhk2bmp(argv[2], argv[3]);
  else {
    showUsage(argv);
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
