#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Renderer.h"
#include "Vec2i.h"


struct SpritePixel {
	SpritePixel(Vec2i pos, const Vec3& rgb, float alpha) :
		pos{pos},
		rgb(rgb),
		alpha(alpha)
	{}
	
	Vec2i pos;
	Vec3 rgb;
	float alpha;
};

class Grid {
public:
	Grid(std::shared_ptr<Renderer> renderer);
	
	void render(const std::vector<SpritePixel>& spritePixels) const;
	void clear();
	void setPixel(uint32_t x, uint32_t y, int8_t val);
	int8_t getPixel(uint32_t x, uint32_t y) const;
	
	std::string toString() const;
	
	uint32_t getWidth() const {return width;}
	uint32_t getHeight() const {return height;}
	
private:
	uint32_t width;
	uint32_t height;
	std::shared_ptr<Renderer> renderer;

	std::vector<int8_t> data;

	size_t gridIndex(uint32_t x, uint32_t y) const;

};
