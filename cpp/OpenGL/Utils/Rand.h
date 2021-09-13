#pragma once

#include <random>
#include <algorithm>

class Random {
public:
  Random();
  Random(uint32_t seed);
  
  float rand005();
  float rand051();
  float rand01();
  float rand11();
  float rand0Pi();
  template <typename T> T rand(T a, T b) {
      return a+T(rand01()*(b-a));
  }
  template <typename T> void shuffle(std::vector<T>& a) {
      for (size_t i=0;i<a.size();++i) {
          size_t r = Random::rand<size_t>(0,i+1);
          std::swap(a[i], a[r]);
      }
  }
  
private:
  std::random_device rd;
  std::mt19937 gen;
  std::uniform_real_distribution<float> dis01;
  std::uniform_real_distribution<float> dis11;
  std::uniform_real_distribution<float> disPi;
  std::uniform_real_distribution<float> dis005;
  std::uniform_real_distribution<float> dis051;

};

extern Random staticRand;
