#include <iostream>
#include <vector>
#include <string>

#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char ** argv) {
  const size_t testCount = 500;
  const size_t vecLength = 1000000;
  
  std::vector<uint64_t> test;
  uint64_t sum = 0;
  uint64_t total = 0;
  test.resize(vecLength);

  auto t1 = Clock::now();
  for (size_t i = 0; i<test.size(); ++i) {
    test[i] = i;
  }
  auto t2 = Clock::now();

  
  for (size_t x = 0;x<3;++x) {
    t1 = Clock::now();
    for (size_t t = 0;t<testCount;++t) {
      sum = 0;
      for (auto iter = test.begin(); iter < test.end(); ++iter) {
        sum += *iter;
      }
      total += sum;
    }
    t2 = Clock::now();
    std::cout << "Iterator:\t" << std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() << " ns passed" << std::endl;

    t1 = Clock::now();
    for (size_t t = 0;t<testCount;++t) {
      sum = 0;
      for (auto v : test) {
        sum += v;
      }
      total += sum;
    }
    t2 = Clock::now();
    std::cout << "Foreach:\t" << std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() << " ns passed" << std::endl;

    sum = 0;
    t1 = Clock::now();
    for (size_t t = 0;t<testCount;++t) {
      sum = 0;
      for (size_t i = 0; i<test.size(); ++i) {
        sum += test[i];
      }
      total += sum;
    }
    t2 = Clock::now();
    std::cout << "Indices:\t" << std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() << " ns passed" << std::endl;

    sum = 0;
    t1 = Clock::now();
    for (size_t t = 0;t<testCount;++t) {
      sum = 0;
      for (size_t i = 0; i<vecLength; ++i) {
        sum += test[i];
      }
      total += sum;
    }
    t2 = Clock::now();
    std::cout << "Size cache:\t" << std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() << " ns passed" << std::endl;

    sum = 0;
    t1 = Clock::now();
    for (size_t t = 0;t<testCount;++t) {
      sum = 0;
      uint64_t* data = test.data();
      for (size_t i = 0; i<vecLength; ++i) {
        sum += *data;
        ++data;
      }
      total += sum;
    }
    t2 = Clock::now();
    std::cout << "Raw Data:\t" << std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() << " ns passed" << std::endl << std::endl;
  }
  
  std::cout << "Total is " << total << std::endl;
  
  return EXIT_SUCCESS;
}

