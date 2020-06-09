#pragma once

#include <vector>
#include <memory>
#include <string>

#include <Vec3.h>
#include <Mat4.h>
#include <GLEnv.h>
#include <GLProgram.h>
#include <Tesselation.h>
#include <GLBuffer.h>
#include <GLArray.h>
#include <GLTexture2D.h>
#include <ParticleSystem.h>
#include <FresnelVisualizer.h>
#include <PlanarMirror.h>

class Scene {
public:
	Scene();
	void render(float t0, const Mat4& v, const Mat4& p, const Dimensions& dim);
	
	void setShowFresnelFrame(bool showFresnelFrame) {this->showFresnelFrame = showFresnelFrame;}
	void setParticleBounce(bool bounce) {particleSystem.setBounce(bounce);}
	void setParticleColors(const Vec3& color) {particleSystem.setColor(color);}
	void setParticleAcceleration(const Vec3& acc) {particleSystem.setAcceleration(acc);}
	void setParticleMaxAge(float maxAge) {particleSystem.setMaxAge(maxAge);}


private:
	Tesselation sphere;
	GLBuffer vbBallPos;
	GLBuffer vbBallNorm;
	GLBuffer vbBallTan;
	GLBuffer vbBallTc;
	GLBuffer ibBall;
	GLArray ballArray;
	GLTexture2D ballAlbedo;
	GLTexture2D ballNormalMap;
	
	Tesselation square;	
	GLBuffer vbWallPos;
	GLBuffer vbWallNorm;
	GLBuffer vbWallTan;
	GLBuffer vbWallTc;
	GLBuffer ibWall;
	GLArray wallArray;
	GLTexture2D brickWallAlbedo;
	GLTexture2D brickWallNormalMap;
	
	GLTexture2D floorAlbedo;
	GLTexture2D floorNormalMap;

	GLTexture2D ceilingAlbedo;
	GLTexture2D ceilingNormalMap;
	
	const GLProgram progNormalMap;
	const GLint mvpLocationNormalMap;
	const GLint mLocationNormalMap;
	const GLint mitLocationNormalMap;
	const GLint invVLocationNormalMap;
	const GLint lpLocationNormalMap;
	const GLint texRescaleLocationNormalMap;
	const GLint texLocationNormalMap;
	const GLint normMapLocationNormalMap;
	
	std::unique_ptr<FresnelVisualizer> fresnelBall;
	
	PlanarMirror mirror;
	std::shared_ptr<SphereStart> starter;
	ParticleSystem particleSystem;
	
	
	bool showFresnelFrame;
	
	
	void checkGLError(const std::string& id);
	
	
	void renderWorld(float t0, const Mat4& m, const Mat4& v, const Mat4& p, const Dimensions& dim);

};