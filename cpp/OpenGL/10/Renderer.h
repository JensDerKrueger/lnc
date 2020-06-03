#pragma once 

#include <vector>
#include <Vec3.h>

class Renderer {
public:
	Renderer(uint32_t width, uint32_t height) :
		w(width),
		h(height)
	{}	
	virtual void render(const std::vector<Vec3>& grid) = 0;
	
	uint32_t width() const {return w;}
	uint32_t height() const {return h;}
	
private:
	uint32_t w;
	uint32_t h;
	uint32_t gridIndex(uint32_t x, uint32_t y);
	
};

