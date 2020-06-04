#pragma once 

#include "Renderer.h"

class TextRenderer : public Renderer {
public:
	TextRenderer(uint32_t width, uint32_t height) : Renderer(width,height) {}
	virtual ~TextRenderer() {}
	virtual void render(const std::vector<Vec3>& grid);
	
private:
	static void setColor(const Vec3& color);
};
	
	
