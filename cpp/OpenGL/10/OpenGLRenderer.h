#pragma once 

#include "Renderer.h"

#include <Vec2.h>
#include <Vec2i.h>
#include <GLProgram.h>
#include <Tesselation.h>
#include <GLBuffer.h>
#include <GLArray.h>
#include <GLTexture2D.h>
#include <ParticleSystem.h>

class OpenGLRenderer : public Renderer {
public:
	OpenGLRenderer(uint32_t width, uint32_t height);
	virtual ~OpenGLRenderer() {}
			
	virtual void render(const std::array<Vec2i,4>& tetrominoPos, const Vec3& currentColor, 
						const std::array<Vec2i,4>& nextTetrominoPos, const Vec3& nextColor,
						const std::array<Vec2i,4>& targerTetrominoPos, float time);
	
	void setViewport(const Dimensions& dim) {this->dim = dim;}
		
	void dropAnimation(const std::array<Vec2i,4>& source, const Vec3& sourceColor, const std::array<Vec2i,4>& target, const std::vector<uint32_t>& clearedRows);
	bool isAnimating() const;
	
private:	
	Tesselation brick;
	GLBuffer vbBrickPos;
	GLBuffer vbBrickNorm;
	GLBuffer vbBrickTan;
	GLBuffer vbBrickTc;
	GLBuffer ibBrick;
	GLArray brickArray;
	GLTexture2D brickAlbedo;
	GLTexture2D brickNormalMap;

	Tesselation background;
	GLBuffer vbBackgroundPos;
	GLBuffer vbBackgroundNorm;
	GLBuffer vbBackgroundTan;
	GLBuffer vbBackgroundTc;
	GLBuffer ibBackground;
	GLArray backgroundArray;
	GLTexture2D backgroundAlbedo;
	GLTexture2D backgroundNormalMap;

	Tesselation pillar;
	GLBuffer vbPillarPos;
	GLBuffer vbPillarNorm;
	GLBuffer vbPillarTan;
	GLBuffer vbPillarTc;
	GLBuffer ibPillar;
	GLArray pillarArray;
	GLTexture2D pillarAlbedo;
	GLTexture2D pillarNormalMap;

	const GLProgram progBrick;
	const GLint mvpLocationBrick;
	const GLint mLocationBrick;
	const GLint mitLocationBrick;
	const GLint invVLocationBrick;	
	const GLint colorBrickLocation;
	const GLint opacityBrickLocation;
	const GLint fractalAnimationLocation;

	const GLProgram progNormalMap;
	const GLint mvpLocationNormalMap;
	const GLint mLocationNormalMap;
	const GLint mitLocationNormalMap;
	const GLint invVLocationNormalMap;
	const GLint lpLocationNormalMap;
	const GLint texLocationNormalMap;
	const GLint normMapLocationNormalMap;
	const GLint colorLocation;
	const GLint opacityLocation;
	
	Dimensions dim;
	
	std::shared_ptr<BrickStart> starter;
	ParticleSystem particleSystem;
	
	Vec2i animationStart;
	Vec2 animationCurrent;
	Vec2i animationTarget;
	double animationStartTime;
	double currentTime;
	std::array<Vec2i,4> droppedTetromino;
	Vec3 droppedTetrominoColor;
	std::vector<uint32_t> clearedRows;
		
	Vec3 pos2Coord(const Vec2& pos, float dist) const;
	
	void clearRows();

};
	
	
