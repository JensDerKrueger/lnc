#pragma once 

#include "Renderer.h"

class TextRenderer : public Renderer {
public:	
	virtual void render(const std::vector<Vec3>& grid);
	
private:
	static void setColor(const Vec3& color);
};
	
	
