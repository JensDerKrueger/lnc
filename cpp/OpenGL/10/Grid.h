#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Renderer.h"
#include "Vec2i.h"

class Grid {
public:
	Grid(std::shared_ptr<Renderer> renderer);
	
	void render(const std::array<Vec2i,4>& tetrominoPos, const Vec3& currentColor, const std::array<Vec2i,4>& nextTetrominoPos, const Vec3& nextColor,
				const std::array<Vec2i,4>& targerTetrominoPos, float time) const;
	void clear();
	void setPixel(size_t x, size_t y, int8_t val);
	int8_t getPixel(size_t x, size_t y) const;
	
	std::string toString() const;
	
	uint32_t getWidth() const {return width;}
	uint32_t getHeight() const {return height;}
	
	std::shared_ptr<Renderer> getRenderer() const {return renderer;}
private:
	uint32_t width;
	uint32_t height;
	std::shared_ptr<Renderer> renderer;

	std::vector<int8_t> data;

	size_t gridIndex(size_t x, size_t y) const;

};
