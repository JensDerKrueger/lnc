#pragma once 

#include <vector>
#include <Vec3.h>
#include <Vec2i.h>

class Renderer {
public:
	Renderer(uint32_t width, uint32_t height) :
		w(width),
		h(height),
		showPreview{true},
		showTarget{true}
	{}	
	virtual void render(const std::array<Vec2i,4>& tetrominoPos, const Vec3& currentColor, 
						const std::array<Vec2i,4>& nextTetrominoPos, const Vec3& nextColor,
						const std::array<Vec2i,4>& targerTetrominoPos,
						const std::vector<Vec3>& colorData, float time) = 0;
	
	uint32_t width() const {return w;}
	uint32_t height() const {return h;}

	uint32_t gridIndex(uint32_t x, uint32_t y) {return x+y*w;}
	
	void setShowPreview(bool showPreview) {this->showPreview = showPreview;}
	bool getShowPreview () const {return showPreview;}

	void setShowTarget(bool showTarget) {this->showTarget = showTarget;}
	bool getShowTarget () const {return showTarget;}

	virtual void clearRows(const std::vector<uint32_t>& rows) = 0;
	virtual void actionCam(const std::array<Vec2i,4>& source, const std::array<Vec2i,4>& target) = 0;
	
private:
	uint32_t w;
	uint32_t h;
	
	bool showPreview;
	bool showTarget;

};

