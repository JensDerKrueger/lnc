#pragma once 

#include <vector>
#include <Vec3.h>
#include <Vec2i.h>

class Renderer {
public:
	Renderer(uint32_t width, uint32_t height) :
		w(width),
		h(height)
	{}	
	virtual void render(const std::array<Vec2i,4>& tetrominoPos, const Vec3& currentColor, 
						const std::array<Vec2i,4>& nextTetrominoPos, const Vec3& nextColor,
						const std::vector<Vec3>& colorData) = 0;
	
	uint32_t width() const {return w;}
	uint32_t height() const {return h;}

	uint32_t gridIndex(uint32_t x, uint32_t y) {return x+y*w;}
	
private:
	uint32_t w;
	uint32_t h;
	
};

