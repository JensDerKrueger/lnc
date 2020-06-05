#pragma once 

#include "Renderer.h"

#include <GLProgram.h>
#include <Tesselation.h>
#include <GLBuffer.h>
#include <GLArray.h>
#include <GLTexture2D.h>

class OpenGLRenderer : public Renderer {
public:
	OpenGLRenderer(uint32_t width, uint32_t height);
	virtual ~OpenGLRenderer() {}
	virtual void render(const std::array<Vec2i,4>& tetrominoPos, const Vec3& currentColor, 
						const std::array<Vec2i,4>& nextTetrominoPos, const Vec3& nextColor, 
						const std::vector<Vec3>& colorData);
	
	void setViewport(const Dimensions& dim) {this->dim = dim;}
	
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


	const GLProgram progNormalMap;
	const GLint mvpLocationNormalMap;
	const GLint mLocationNormalMap;
	const GLint mitLocationNormalMap;
	const GLint invVLocationNormalMap;
	const GLint lpLocationNormalMap;
	const GLint texRescaleLocationNormalMap;
	const GLint texLocationNormalMap;
	const GLint normMapLocationNormalMap;
	const GLint colorLocation;
	
	Dimensions dim;
};
	
	
