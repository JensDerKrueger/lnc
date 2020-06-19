#include <algorithm>
#include <iostream>
#include <sstream>

#include "Grid.h"

#include "TetrisConst.h"


Grid::Grid(std::shared_ptr<Renderer> renderer) :
width{renderer->width()},
height{renderer->height()},
renderer{renderer}	
{
	clear();
}

void Grid::render(const std::array<Vec2i,4>& tetrominoPos, const Vec3& currentColor, 
				  const std::array<Vec2i,4>& nextTetrominoPos, const Vec3& nextColor,
				  const std::array<Vec2i,4>& targerTetrominoPos, float time) const {
	if (!renderer->isAnimating()) {
		std::vector<Vec3> colorData(width*height);
		std::transform(data.begin(), data.end(), colorData.begin(), 
						[](int8_t v) -> Vec3 { return v == -1 ? Vec3(-1,-1,-1) : colors[v]; });
		renderer->setObstacles(colorData);
	}
	renderer->render(tetrominoPos, currentColor, nextTetrominoPos, nextColor, targerTetrominoPos, time);
}

void Grid::clear() {
    data.clear();
    data.resize(width*height,-1);
}

void Grid::setPixel(uint32_t x, uint32_t y, int8_t val) {
	data[gridIndex(x,y)] = val;
}

int8_t Grid::getPixel(uint32_t x, uint32_t y) const {
	return data[gridIndex(x,y)];
}

std::string Grid::toString() const {
	size_t i = 0;
	std::stringstream s;
	for (uint32_t y = 0;y < height; ++y) {
		for (uint32_t x = 0;x < width; ++x) {
			s << data[i++] << " ";
		}
		s << std::endl;
	}
	return s.str();
}

size_t Grid::gridIndex(uint32_t x, uint32_t y) const {
	return x+width*y;
}
