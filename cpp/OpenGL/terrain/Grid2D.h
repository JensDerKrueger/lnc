#pragma once

#include <iostream>
#include <string>
#include <vector>

class Grid2D {
public:
    Grid2D(size_t width, size_t height);
    
    size_t getWidth() const;
    size_t getHeight() const;
    std::string toString() const;
    std::vector<uint8_t> toByteArray() const;
    
    void setValue(size_t x, size_t y, float value);
    float getValue(size_t x, size_t y) const;
    float sample(float x, float y) const ;
    static Grid2D genRandom(size_t x, size_t y);
    Grid2D operator*(const float& value) const;
    Grid2D operator/(const float& value) const;
    Grid2D operator+(const Grid2D& other) const;
    void normalize();

    friend std::ostream& operator<<(std::ostream &os, const Grid2D& v);

private:
    size_t width;
    size_t height;
    std::vector<float> data{};
    size_t index(size_t x, size_t y) const;
};
