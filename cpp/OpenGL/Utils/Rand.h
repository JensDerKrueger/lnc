#pragma once

#include <random>
#include <algorithm>

class Rand {    
public:
    static float rand005();
    static float rand051();
    static float rand01();
    static float rand11();
    static float rand0Pi();
    template <typename T> static T rand(T a, T b) {
        return a+T(rand01()*(b-a));
    }
    template <typename T> static void shuffle(std::vector<T>& a) {
        for (size_t i=0;i<a.size();++i) {
            size_t r = rand<T>(0,i+1);
            std::swap(a[i], a[r]);
        }
    }
private:
    static std::random_device rd;
    static std::mt19937 gen;
    static std::uniform_real_distribution<float> dis01;
    static std::uniform_real_distribution<float> dis11;
    static std::uniform_real_distribution<float> disPi;
    static std::uniform_real_distribution<float> dis005;
    static std::uniform_real_distribution<float> dis051;

};
