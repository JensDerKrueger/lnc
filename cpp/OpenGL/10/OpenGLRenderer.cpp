#include <iostream>
#include <cmath>

#include "OpenGLRenderer.h"

#include <bmp.h>

OpenGLRenderer::OpenGLRenderer(uint32_t width, uint32_t height) : 
	Renderer(width,height),
	brick{Tesselation::genBrick({0,0,0}, {0.9,0.9,0.9})},
	vbBrickPos{GL_ARRAY_BUFFER},
	vbBrickNorm{GL_ARRAY_BUFFER},
	vbBrickTan{GL_ARRAY_BUFFER},
	vbBrickTc{GL_ARRAY_BUFFER},
	ibBrick{GL_ELEMENT_ARRAY_BUFFER},
	brickArray{},
	brickAlbedo{GL_LINEAR, GL_LINEAR},
	brickNormalMap{GL_LINEAR, GL_LINEAR},
	background{Tesselation::genBrick({0,0,0}, {float(width),float(height),1}, {float(width/10),float(height/10),1.0f/10})},
	vbBackgroundPos{GL_ARRAY_BUFFER},
	vbBackgroundNorm{GL_ARRAY_BUFFER},
	vbBackgroundTan{GL_ARRAY_BUFFER},
	vbBackgroundTc{GL_ARRAY_BUFFER},
	ibBackground{GL_ELEMENT_ARRAY_BUFFER},
	backgroundArray{},
	backgroundAlbedo{GL_LINEAR, GL_LINEAR},
	backgroundNormalMap{GL_LINEAR, GL_LINEAR},
	pillar{Tesselation::genBrick({0,0,0}, {1,float(height),2}, {1.0f/10.f,float(height/10),1.0f/10})},
	vbPillarPos{GL_ARRAY_BUFFER},
	vbPillarNorm{GL_ARRAY_BUFFER},
	vbPillarTan{GL_ARRAY_BUFFER},
	vbPillarTc{GL_ARRAY_BUFFER},
	ibPillar{GL_ELEMENT_ARRAY_BUFFER},
	pillarArray{},
	pillarAlbedo{GL_LINEAR, GL_LINEAR},
	pillarNormalMap{GL_LINEAR, GL_LINEAR},
	progBrick{GLProgram::createFromFile("brickVertex.glsl", "brickFragment.glsl")},
	mvpLocationBrick{progBrick.getUniformLocation("MVP")},
	mLocationBrick{progBrick.getUniformLocation("M")},
	mitLocationBrick{progBrick.getUniformLocation("Mit")},
	invVLocationBrick{progBrick.getUniformLocation("invV")},
	colorBrickLocation{progBrick.getUniformLocation("color")},
	opacityBrickLocation{progBrick.getUniformLocation("opacity")},
	fractalAnimationLocation{progBrick.getUniformLocation("animation")},	
	progNormalMap{GLProgram::createFromFile("backgroundVertex.glsl", "backgroundFragment.glsl")},
	mvpLocationNormalMap{progNormalMap.getUniformLocation("MVP")},
	mLocationNormalMap{progNormalMap.getUniformLocation("M")},
	mitLocationNormalMap{progNormalMap.getUniformLocation("Mit")},
	invVLocationNormalMap{progNormalMap.getUniformLocation("invV")},
	lpLocationNormalMap{progNormalMap.getUniformLocation("vLightPos")},
	texLocationNormalMap{progNormalMap.getUniformLocation("textureSampler")},
	normMapLocationNormalMap{progNormalMap.getUniformLocation("normalSampler")},
	colorLocation{progNormalMap.getUniformLocation("color")},
	opacityLocation{progNormalMap.getUniformLocation("opacity")},
	starter(std::make_shared<BrickStart>(Vec3{0,0,0},Vec3{0,0,0})),
	particleSystem{8000, starter, {-10,-10,50}, {10,10,55}, {0,0,0}, Vec3{-100.0f,-100.0f,-100.0f}, Vec3{100.0f,100.0f,100.0f}, 5.0f, 80.0f, RAINBOW_COLOR, false}
{
	vbBrickPos.setData(brick.getVertices(),3);
	vbBrickNorm.setData(brick.getNormals(),3);
	vbBrickTan.setData(brick.getTangents(),3);
	vbBrickTc.setData(brick.getTexCoords(),2);	
	ibBrick.setData(brick.getIndices());	
	
	brickArray.bind();
	brickArray.connectVertexAttrib(vbBrickPos,progBrick,"vPos",3);
	brickArray.connectVertexAttrib(vbBrickNorm,progBrick,"vNorm",3);
	brickArray.connectVertexAttrib(vbBrickTc,progBrick,"vTc",2);	
	brickArray.connectIndexBuffer(ibBrick);	

	vbBackgroundPos.setData(background.getVertices(),3);
	vbBackgroundNorm.setData(background.getNormals(),3);
	vbBackgroundTan.setData(background.getTangents(),3);
	vbBackgroundTc.setData(background.getTexCoords(),2);	
	ibBackground.setData(background.getIndices());
	BMP::Image backgroundAlbedoImage{BMP::load("BackgroundAlbedo.bmp")};
	backgroundAlbedo.setData(backgroundAlbedoImage.data, backgroundAlbedoImage.width, backgroundAlbedoImage.height, backgroundAlbedoImage.componentCount);
	BMP::Image backgroundNormalImage{BMP::load("BackgroundNormal.bmp")};
	backgroundNormalMap.setData(backgroundNormalImage.data, backgroundNormalImage.width, backgroundNormalImage.height, backgroundNormalImage.componentCount);	
	
	backgroundArray.bind();
	backgroundArray.connectVertexAttrib(vbBackgroundPos,progNormalMap,"vPos",3);
	backgroundArray.connectVertexAttrib(vbBackgroundNorm,progNormalMap,"vNorm",3);
	backgroundArray.connectVertexAttrib(vbBackgroundTan,progNormalMap,"vTan",3);
	backgroundArray.connectVertexAttrib(vbBackgroundTc,progNormalMap,"vTc",2);
	backgroundArray.connectIndexBuffer(ibBackground);

	vbPillarPos.setData(pillar.getVertices(),3);
	vbPillarNorm.setData(pillar.getNormals(),3);
	vbPillarTan.setData(pillar.getTangents(),3);
	vbPillarTc.setData(pillar.getTexCoords(),2);	
	ibPillar.setData(pillar.getIndices());
	BMP::Image pillarAlbedoImage{BMP::load("BackgroundAlbedo.bmp")};
	pillarAlbedo.setData(pillarAlbedoImage.data, pillarAlbedoImage.width, pillarAlbedoImage.height, pillarAlbedoImage.componentCount);
	BMP::Image pillarNormalImage{BMP::load("BackgroundNormal.bmp")};
	pillarNormalMap.setData(pillarNormalImage.data, pillarNormalImage.width, pillarNormalImage.height, pillarNormalImage.componentCount);	
	
	pillarArray.bind();
	pillarArray.connectVertexAttrib(vbPillarPos,progNormalMap,"vPos",3);
	pillarArray.connectVertexAttrib(vbPillarNorm,progNormalMap,"vNorm",3);
	pillarArray.connectVertexAttrib(vbPillarTan,progNormalMap,"vTan",3);
	pillarArray.connectVertexAttrib(vbPillarTc,progNormalMap,"vTc",2);
	pillarArray.connectIndexBuffer(ibPillar);


}

Vec3 OpenGLRenderer::pos2Coord(const Vec2& pos, float dist) const {
	return {float(pos.x())-float(width())/2+0.5f,
			float(height()-pos.y())-float(height()/2)-0.5f,
			-dist};
}


void OpenGLRenderer::clearRows(const std::vector<uint32_t>& rows) {
	const size_t particlesPerRow{((1<<rows.size())*200)/rows.size()};
	
	for (uint32_t row : rows) {
		Vec3 coord{pos2Coord(Vec2i(width()/2.0,row), 20.0f)};
		starter->setStart(coord, Vec3(width(),1.0f,1.0f));
		particleSystem.restart(particlesPerRow);
	}
}

void OpenGLRenderer::actionCam(const std::array<Vec2i,4>& source, const std::array<Vec2i,4>& target) {
	animationStart = source[0];
	animationCurrent = animationStart;
	animationTarget = target[0];
	animationStartTime = currentTime;
}

bool OpenGLRenderer::isAnimating() const {
	return animationCurrent != animationTarget;
}

void OpenGLRenderer::render(const std::array<Vec2i,4>& tetrominoPos, const Vec3& currentColor,
							const std::array<Vec2i,4>& nextTetrominoPos, const Vec3& nextColor,
							const std::array<Vec2i,4>& targerTetrominoPos, float time) {
								
	currentTime = time;
								
	// setup basic OpenGL states that do not change during the frame
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);    
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	Vec3 lookAtVec;
	Vec3 lookFromVec;
	Vec3 upVec;
	Mat4 v;

	if (animationCurrent == animationTarget) {
		lookFromVec = Vec3{0,0,5};
		lookAtVec = Vec3{0,0,0};
		upVec = Vec3{0,1,0};
		v = Mat4{Mat4::lookAt(lookFromVec,lookAtVec,upVec)};
	} else {
		const float totalTime = float(animationTarget.y() - animationStart.y())*0.08;
		float a = (currentTime - animationStartTime)/totalTime;
		if (a < 1)
			animationCurrent = Vec2(animationStart) * (1-a) + Vec2(animationTarget) * a;
		else 
			animationCurrent = animationTarget;

		lookFromVec = Vec3{pos2Coord(animationCurrent, 20.0f)};
		lookAtVec = lookFromVec+Vec3(0,-10,0);
		upVec = Vec3{0,0,1};
		v = Mat4{Mat4::lookAt(lookFromVec,lookAtVec,upVec)};
	}

	// setup viewport and clear buffers
	glViewport(0, 0, dim.width, dim.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const Mat4 p{Mat4::perspective(45, dim.aspect(), 0.0001, 1000)};
	
	progNormalMap.enable();
	Vec3 lightPos{Mat4::rotationZ(time*20) * Vec3{0,10,-4}};
	progNormalMap.setUniform(lpLocationNormalMap, lightPos);
	progNormalMap.setTexture(normMapLocationNormalMap,backgroundNormalMap,0);
	progNormalMap.setTexture(texLocationNormalMap,backgroundAlbedo,1);
	progNormalMap.setUniform(opacityLocation, 1.0f);
	
	backgroundArray.bind();
	
	Mat4 m{Mat4::translation(Vec3{0,0,-21.0f })};
	progNormalMap.setUniform(mvpLocationNormalMap, m*v*p);
	progNormalMap.setUniform(mLocationNormalMap, m);
	progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(m), true);
	progNormalMap.setUniform(invVLocationNormalMap, Mat4::inverse(v)); 
	progNormalMap.setUniform(colorLocation, Vec3{0.5,0.5,0.5});
	glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);


	pillarArray.bind();

	
	m = Mat4{Mat4::translation(Vec3{float(width())/2.0f+0.5f,0,-20.0f })};
	progNormalMap.setUniform(mvpLocationNormalMap, m*v*p);
	progNormalMap.setUniform(mLocationNormalMap, m);
	progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(m), true);
	progNormalMap.setUniform(invVLocationNormalMap, Mat4::inverse(v)); 
	progNormalMap.setUniform(colorLocation, Vec3{0.5,0.5,0.5});

	glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);

	m = Mat4{Mat4::translation(Vec3{-(float(width())/2.0f+0.5f),0,-20.0f })};
	progNormalMap.setUniform(mvpLocationNormalMap, m*v*p);
	progNormalMap.setUniform(mLocationNormalMap, m);
	progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(m), true);
	progNormalMap.setUniform(invVLocationNormalMap, Mat4::inverse(v)); 
	progNormalMap.setUniform(colorLocation, Vec3{0.5,0.5,0.5});

	glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);



	progBrick.enable();
	brickArray.bind();
	progBrick.setUniform(opacityBrickLocation, 1.0f);
	progBrick.setUniform(invVLocationBrick, Mat4::inverse(v)); 
	progBrick.setUniform(fractalAnimationLocation, time);

	uint32_t i{0};
	for (uint32_t y = 0;y < height();++y) {
		for (uint32_t x = 0;x < width();++x) {
			const Vec3 c = getObstacles()[i++];
			
			if (c.r() < 0) continue; // empty spaces
			
			const Mat4 m{Mat4::translation(pos2Coord(Vec2i(x,y), 20.0f))};
			progBrick.setUniform(mvpLocationBrick, m*v*p);
			progBrick.setUniform(mLocationBrick, m);
			progBrick.setUniform(mitLocationBrick, Mat4::inverse(m), true);			
			progBrick.setUniform(colorBrickLocation, c);
			glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
		}
	}
	
	for (const Vec2i& pos : tetrominoPos) {
		const Mat4 m{Mat4::translation(pos2Coord(pos, 20.0f))};
		progBrick.setUniform(mvpLocationBrick, m*v*p);
		progBrick.setUniform(mLocationBrick, m);
		progBrick.setUniform(mitLocationBrick, Mat4::inverse(m), true);			
		progBrick.setUniform(colorBrickLocation, currentColor);
		glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
	}	

	if (getShowPreview()) {
		for (const Vec2i& pos : nextTetrominoPos) {
			const Mat4 m{Mat4::translation(pos2Coord(pos+Vec2i(13,17), 20.0f))};
			progBrick.setUniform(mvpLocationBrick, m*v*p);
			progBrick.setUniform(mLocationBrick, m);
			progBrick.setUniform(mitLocationBrick, Mat4::inverse(m), true);			
			progBrick.setUniform(colorBrickLocation, nextColor);
			glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
		}	
	}
	
	if (getShowTarget()) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
		glDepthMask(GL_FALSE);
			
		for (const Vec2i& pos : targerTetrominoPos) {
			const Mat4 m{Mat4::translation(pos2Coord(pos, 20.0f))};
			progBrick.setUniform(mvpLocationBrick, m*v*p);
			progBrick.setUniform(mLocationBrick, m);
			progBrick.setUniform(mitLocationBrick, Mat4::inverse(m), true);			
			progBrick.setUniform(colorBrickLocation, currentColor);
			progBrick.setUniform(opacityBrickLocation, 0.2f);
			glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
		}

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
	
	particleSystem.setSize(dim.height/30);		
	particleSystem.render(v,p);
	particleSystem.update(time);
}