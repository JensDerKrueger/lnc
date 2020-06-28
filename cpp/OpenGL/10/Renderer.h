#pragma once 

#include <vector>
#include <Vec3.h>
#include <Vec2.h>

class Renderer {
public:
	Renderer(uint32_t width, uint32_t height) :
		w(width),
		h(height),
        backgroundParam(0),
		showPreview{true},
		showTarget{true}
	{}	
	
	void setObstacles(const std::vector<Vec3>& obstacleData) {this->obstacleData = obstacleData;}
	std::vector<Vec3> getObstacles() {return obstacleData;}
	
	virtual void render(const std::array<Vec2i,4>& tetrominoPos, const Vec3& currentColor, 
						const std::array<Vec2i,4>& nextTetrominoPos, const Vec3& nextColor,
						const std::array<Vec2i,4>& targerTetrominoPos, float time) = 0;
	
	uint32_t width() const {return w;}
	uint32_t height() const {return h;}

	uint32_t gridIndex(uint32_t x, uint32_t y) {return x+y*w;}
	
	void setShowPreview(bool showPreview) {this->showPreview = showPreview;}
	bool getShowPreview () const {return showPreview;}

	void setShowTarget(bool showTarget) {this->showTarget = showTarget;}
	bool getShowTarget () const {return showTarget;}

	virtual void dropAnimation(const std::array<Vec2i,4>& source, const Vec3& sourceColor, const std::array<Vec2i,4>& target, const std::vector<uint32_t>& clearedRows) = 0;
	virtual bool isAnimating() const = 0;
	
    void incBackgroundParam() {backgroundParam += 0.01f;}
    void decBackgroundParam() {if (backgroundParam>=0) backgroundParam -= 0.01f;}
    float getBackgroundParam() const {return backgroundParam;}
    void setBackgroundParam(float backgroundParam) {this->backgroundParam = backgroundParam;}
    
    virtual void setGameOver(bool gameOver, uint32_t score) = 0;

private:
	uint32_t w;
	uint32_t h;
    float backgroundParam;
	
	bool showPreview;
	bool showTarget;

	std::vector<Vec3> obstacleData;

};

