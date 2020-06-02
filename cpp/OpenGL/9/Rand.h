#pragma once

#include <random>

class Rand {    
public:
    static float rand005();
    static float rand051();
    static float rand01();
    static float rand11();
    static float rand0Pi();
private:
    static std::random_device rd;
    static std::mt19937 gen;
    static std::uniform_real_distribution<float> dis01;
    static std::uniform_real_distribution<float> dis11;
    static std::uniform_real_distribution<float> disPi;
    static std::uniform_real_distribution<float> dis005;
    static std::uniform_real_distribution<float> dis051;

};
