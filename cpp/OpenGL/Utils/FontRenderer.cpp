#include "FontRenderer.h"

#include <string>

BMP::Image FontRenderer::renderNumber(uint32_t number) {

    BMP::Image numberSource = BMP::load("numbers.bmp");
    std::vector<std::pair<Vec2ui, Vec2ui>> positions;
    for (uint32_t i = 0;i<10;++i) {
        positions.push_back(std::make_pair<Vec2ui, Vec2ui>({100*i,0}, {100*(i+1)-1,145}));
    }
    
    std::vector<uint32_t> digits;
    if (number == 0) {
        digits.push_back(0);
    } else {
        while (number > 0) {
            digits.push_back(number%10);
            number = number/10;
        }
        std::reverse(digits.begin(), digits.end());
    }
    
    
    Vec2ui dims{0,0};
    for (uint32_t digit : digits) {
        const auto& pos = positions[digit];
        Vec2ui size = pos.second-pos.first;
        dims = Vec2ui{dims.x()+size.x(), std::max(dims.y(), size.y())};
    }
    
    BMP::Image result{dims.x(),dims.y(),numberSource.componentCount,
        std::vector<uint8_t>(dims.x()*dims.y()*numberSource.componentCount)};
    
    Vec2ui currentPos{0,0};
    for (uint32_t digit : digits) {
        const auto& pos = positions[digit];
        Vec2ui size = pos.second-pos.first;
        BMP::blit(numberSource, pos.first, pos.second, result, currentPos);
        currentPos = Vec2ui(currentPos.x()+size.x(), currentPos.y());
    }
    
    return result;
}
