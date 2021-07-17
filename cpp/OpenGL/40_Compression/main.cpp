#include <iostream>

#include "Compression.h"

static void showUsage(char** argv) {
  std::cout << "Usage:" << std::endl;
  std::cout << "\t compress a file " << argv[0] << " c input output" << std::endl;
  std::cout << "\t uncompress a file " << argv[0] << " u input output" << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 4) {
    showUsage(argv);
    return EXIT_FAILURE;
  }
  
  if (std::string{argv[1]} == "c")
    Compression::huffmanEncode(argv[2], argv[3]);
  else if (std::string{argv[1]} == "u")
    Compression::huffmanDecode(argv[2], argv[3]);
  else {
    showUsage(argv);
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
