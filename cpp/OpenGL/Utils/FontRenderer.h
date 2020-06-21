#pragma once

#include <vector>
#include <utility>

#include "bmp.h"
#include "Vec2i.h"



namespace FontRenderer {
    BMP::Image renderNumber(uint32_t number, const std::string& imageFilename, const std::string& positionFilename);
}
