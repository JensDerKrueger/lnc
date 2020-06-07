#include <iostream>

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
	background{Tesselation::genBrick({0,0,0}, {float(width),float(height),1}, {float(width/10),float(height/10),1})},
	vbBackgroundPos{GL_ARRAY_BUFFER},
	vbBackgroundNorm{GL_ARRAY_BUFFER},
	vbBackgroundTan{GL_ARRAY_BUFFER},
	vbBackgroundTc{GL_ARRAY_BUFFER},
	ibBackground{GL_ELEMENT_ARRAY_BUFFER},
	backgroundArray{},
	backgroundAlbedo{GL_LINEAR, GL_LINEAR},
	backgroundNormalMap{GL_LINEAR, GL_LINEAR},
	progNormalMap{GLProgram::createFromFile("normalMapVertex.glsl", "normalMapFragment.glsl")},
	mvpLocationNormalMap{progNormalMap.getUniformLocation("MVP")},
	mLocationNormalMap{progNormalMap.getUniformLocation("M")},
	mitLocationNormalMap{progNormalMap.getUniformLocation("Mit")},
	invVLocationNormalMap{progNormalMap.getUniformLocation("invV")},
	lpLocationNormalMap{progNormalMap.getUniformLocation("vLightPos")},
	texLocationNormalMap{progNormalMap.getUniformLocation("textureSampler")},
	normMapLocationNormalMap{progNormalMap.getUniformLocation("normalSampler")},
	colorLocation{progNormalMap.getUniformLocation("color")},
	opacityLocation{progNormalMap.getUniformLocation("opacity")}
{
	vbBrickPos.setData(brick.getVertices(),3);
	vbBrickNorm.setData(brick.getNormals(),3);
	vbBrickTan.setData(brick.getTangents(),3);
	vbBrickTc.setData(brick.getTexCoords(),2);	
	ibBrick.setData(brick.getIndices());
	BMP::Image brickAlbedoImage{BMP::load("BrickAlbedo.bmp")};
	brickAlbedo.setData(brickAlbedoImage.data, brickAlbedoImage.width, brickAlbedoImage.height, brickAlbedoImage.componentCount);
	BMP::Image brickNormalImage{BMP::load("BrickNormal.bmp")};
	brickNormalMap.setData(brickNormalImage.data, brickNormalImage.width, brickNormalImage.height, brickNormalImage.componentCount);	
	
	brickArray.bind();
	brickArray.connectVertexAttrib(vbBrickPos,progNormalMap,"vPos",3);
	brickArray.connectVertexAttrib(vbBrickNorm,progNormalMap,"vNorm",3);
	brickArray.connectVertexAttrib(vbBrickTan,progNormalMap,"vTan",3);
	brickArray.connectVertexAttrib(vbBrickTc,progNormalMap,"vTc",2);
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

}

Vec3 OpenGLRenderer::pos2Coord(const Vec2i& pos, float dist) const {
	return {float(pos.x())-float(width())/2+0.5f,
			float(height()-pos.y())-float(height()/2)-0.5f,
			-dist};
}

void OpenGLRenderer::render(const std::array<Vec2i,4>& tetrominoPos, const Vec3& currentColor,
							const std::array<Vec2i,4>& nextTetrominoPos, const Vec3& nextColor,
							const std::array<Vec2i,4>& targerTetrominoPos,
							const std::vector<Vec3>& colorData) {
								
	// setup basic OpenGL states that do not change during the frame
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);    
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	const Vec3 lookFromVec{Vec3{0,0,5}};
	const Vec3 lookAtVec{0,0,0};
	const Vec3 upVec{0,1,0};
	const Mat4 v{Mat4::lookAt(lookFromVec,lookAtVec,upVec)};

	// setup viewport and clear buffers
	glViewport(0, 0, dim.width, dim.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const Mat4 p{Mat4::perspective(45, dim.aspect(), 0.0001, 1000)};

	progNormalMap.enable();
	Vec3 lightPos{Vec3{0,0,-4}};
	progNormalMap.setUniform(lpLocationNormalMap, lightPos);
	progNormalMap.setTexture(normMapLocationNormalMap,backgroundNormalMap,0);
	progNormalMap.setTexture(texLocationNormalMap,backgroundAlbedo,1);
	progNormalMap.setUniform(opacityLocation, 1.0f);
	
	backgroundArray.bind();
	const Mat4 m{Mat4::translation(Vec3{0,0,-20.5f})};
	progNormalMap.setUniform(mvpLocationNormalMap, m*v*p);
	progNormalMap.setUniform(mLocationNormalMap, m);
	progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(m), true);
	progNormalMap.setUniform(invVLocationNormalMap, Mat4::inverse(v));
	progNormalMap.setUniform(colorLocation, Vec3{0.5,0.5,0.5});
	glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);

	progNormalMap.setTexture(normMapLocationNormalMap,brickNormalMap,0);
	progNormalMap.setTexture(texLocationNormalMap,brickAlbedo,1);
	
	brickArray.bind();

	uint32_t i{0};
	for (uint32_t y = 0;y < height();++y) {
		for (uint32_t x = 0;x < width();++x) {
			const Vec3 c = colorData[i++];
			
			if (c.r() < 0) continue; // empty spaces
			
			const Mat4 m{Mat4::translation(pos2Coord(Vec2i(x,y), 20.0f))};
			progNormalMap.setUniform(mvpLocationNormalMap, m*v*p);
			progNormalMap.setUniform(mLocationNormalMap, m);
			progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(m), true);
			progNormalMap.setUniform(invVLocationNormalMap, Mat4::inverse(v));
			progNormalMap.setUniform(colorLocation, c);
			glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
		}
	}
	
	for (const Vec2i& pos : tetrominoPos) {
		const Mat4 m{Mat4::translation(pos2Coord(pos, 20.0f))};
		progNormalMap.setUniform(mvpLocationNormalMap, m*v*p);
		progNormalMap.setUniform(mLocationNormalMap, m);
		progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(m), true);
		progNormalMap.setUniform(invVLocationNormalMap, Mat4::inverse(v));
		progNormalMap.setUniform(colorLocation, currentColor);
		glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
	}	

	for (const Vec2i& pos : nextTetrominoPos) {
		const Mat4 m{Mat4::translation(pos2Coord(pos+Vec2i(13,17), 20.0f))};
		progNormalMap.setUniform(mvpLocationNormalMap, m*v*p);
		progNormalMap.setUniform(mLocationNormalMap, m);
		progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(m), true);
		progNormalMap.setUniform(invVLocationNormalMap, Mat4::inverse(v));
		progNormalMap.setUniform(colorLocation, nextColor);
		glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
	}	
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
	glDepthMask(GL_FALSE);
		
	for (const Vec2i& pos : targerTetrominoPos) {
		const Mat4 m{Mat4::translation(pos2Coord(pos, 20.0f))};
		progNormalMap.setUniform(mvpLocationNormalMap, m*v*p);
		progNormalMap.setUniform(mLocationNormalMap, m);
		progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(m), true);
		progNormalMap.setUniform(invVLocationNormalMap, Mat4::inverse(v));
		progNormalMap.setUniform(colorLocation, currentColor);
		progNormalMap.setUniform(opacityLocation, 0.2f);
		glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}