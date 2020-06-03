#include <iostream>

#include "TextRenderer.h"

void TextRenderer::render(const std::vector<Vec3>& grid) {
	std::cout << "\033[2J\033[;H";
	uint32_t i{0};
	for (uint32_t y = 0;y < height();++y) {
		for (uint32_t x = 0;x < width();++x) {
			setColor(grid[i++]);
			std::cout << "  ";
		}
		setColor(Vec3{0,0,0});
		std::cout << "\n";
	}
	std::cout << std::flush;
}

void TextRenderer::setColor(const Vec3& color) {
	const uint32_t index = 16 + uint32_t(color.r()*5) + 
							6 * uint32_t(color.g()*5) + 
						   36 * uint32_t(color.b()*5);
							
	std::cout << "\033[48;5;" << index << "m";
}
