#include <iostream>
#include <string>
#include <vector>

#include "Student.h"


int main(int argc, char** argv) {
      
  std::vector<int> test(20);

    
  
  Student p("Jens", "Krüger", 214446);
    
  std::cout << test.at(10) << std::endl;
  return EXIT_SUCCESS;
}
