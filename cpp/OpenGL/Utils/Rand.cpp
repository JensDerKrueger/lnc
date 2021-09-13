#include "Rand.h"

#ifndef M_PI
constexpr float M_PI = 3.14159265358979323846f;
#endif

Random::Random() :
rd{},
gen{rd()},
dis01{0.0f, 1.0f},
dis11{-1.0f, 1.0f},
disPi{0.0f, 2.0f * float(M_PI)},
dis005{0.0f, 0.5f},
dis051{0.5f, 1.0f}
{
  
}

Random::Random(uint32_t seed)  :
rd{},
gen{seed},
dis01{0.0f, 1.0f},
dis11{-1.0f, 1.0f},
disPi{0.0f, 2.0f * float(M_PI)},
dis005{0.0f, 0.5f},
dis051{0.5f, 1.0f}
{
}



float Random::rand01() {
    return dis01(gen);
}

float Random::rand005() {
    return dis005(gen);
}

float Random::rand051() {
    return dis051(gen);
}

float Random::rand11() {
    return dis11(gen);
}

float Random::rand0Pi() {
    return disPi(gen);
}

Random staticRand;
