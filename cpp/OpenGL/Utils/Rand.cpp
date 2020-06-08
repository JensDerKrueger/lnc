#include "Rand.h"

#ifndef M_PI
constexpr float M_PI = 3.14159265358979323846f;
#endif

std::random_device Rand::rd{};
std::mt19937 Rand::gen{rd()};
std::uniform_real_distribution<float> Rand::dis01{0.0f, 1.0f};
std::uniform_real_distribution<float> Rand::dis005{0.0f, 0.5f};
std::uniform_real_distribution<float> Rand::dis051{0.5f, 1.0f};
std::uniform_real_distribution<float> Rand::dis11{-1.0f, 1.0f};
std::uniform_real_distribution<float> Rand::disPi{0.0f, 2.0f * M_PI};

float Rand::rand01() {
    return dis01(gen);
}

float Rand::rand005() {
    return dis005(gen);
}

float Rand::rand051() {
    return dis051(gen);
}

float Rand::rand11() {
    return dis11(gen);
}

float Rand::rand0Pi() {
    return disPi(gen);
}