#include <string>
#include <fstream>
#include <sstream>

#include "FontRenderer.h"


BMP::Image FontRenderer::renderNumber(uint32_t number,
                                      const std::string& imageFilename,
                                      const std::string& positionFilename) {

    BMP::Image numberSource = BMP::load(imageFilename);
    
    std::vector<std::pair<Vec2ui, Vec2ui>> positions;
    std::ifstream posfile (positionFilename);
    std::string line;
    if (posfile.is_open()) {
        while ( getline (posfile,line) ) {
            std::vector<uint32_t> vals;
            std::stringstream tokenizer(line);
            std::string token;
            while(getline(tokenizer, token, ' ')) {
                vals.push_back(uint32_t(stoi(token)));
            }
            if (vals.size() == 4)
                positions.push_back(std::make_pair<Vec2ui, Vec2ui>({vals[0],vals[1]}, {vals[2],vals[3]}));
        }
        posfile.close();
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
