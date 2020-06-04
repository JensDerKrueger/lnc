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

void Grid::render(const std::vector<SpritePixel>& spritePixels) const {
	std::vector<Vec3> colorData(width*height);

	std::transform(data.begin(), data.end(), colorData.begin(), 
					[](int8_t v) -> Vec3 { return colors[v]; });

	for (const auto& p : spritePixels) {
		size_t index = gridIndex(p.pos.x(), p.pos.y());
		colorData[index] = colorData[index] * (1-p.alpha) + p.rgb * p.alpha;
	}

	renderer->render(colorData);
}

void Grid::clear() {
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

