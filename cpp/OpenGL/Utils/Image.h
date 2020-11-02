#pragma once

struct Image {
    uint32_t width;
    uint32_t height;
    uint32_t componentCount;
    std::vector<uint8_t> data;
    
    size_t computeIndex(uint32_t x, uint32_t y, uint32_t component) const {
        return component+(x+y*width)*componentCount;
    }
    
    uint8_t getValue(uint32_t x, uint32_t y, uint32_t component) const {
        return data[computeIndex(x, y, component)];
    }

    void setValue(uint32_t x, uint32_t y, uint32_t component, uint8_t value) {
        data[computeIndex(x, y, component)] = value;
    }

};
