#include <iostream>
#include <vector>
#include <string>

#include <chrono>
typedef std::chrono::high_resolution_clock Clock;


struct TestStruct {
  TestStruct(const std::vector<uint64_t>& v) :
    foo(v)
  {
  }
  std::vector<uint64_t> foo;
};

int main(int argc, char ** argv) {
  const size_t repetitions = 3;
  const size_t testCount = 500;
  const size_t vecLength = 100;
  
  std::vector<uint64_t> foo(10000);
  
  std::vector<TestStruct> test;


  auto t1 = Clock::now();
  auto t2 = Clock::now();

  for (size_t x = 0;x<repetitions;++x) {
    test.clear();
    t1 = Clock::now();
    for (size_t t = 0;t<testCount;++t) {
      for (size_t i = 0; i<vecLength; ++i) {
        test.push_back(TestStruct(foo));
      }
    }
    t2 = Clock::now();
    std::cout << "Push Back:\t" << std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() << " ns passed" << std::endl;
    
    test.clear();
    t1 = Clock::now();
    for (size_t t = 0;t<testCount;++t) {
      for (size_t i = 0; i<vecLength; ++i) {
        test.emplace_back(foo);
      }
    }
    t2 = Clock::now();
    std::cout << "Emplace Back:\t" << std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() << " ns passed" << std::endl << std::endl;
  }


  return EXIT_SUCCESS;
}

