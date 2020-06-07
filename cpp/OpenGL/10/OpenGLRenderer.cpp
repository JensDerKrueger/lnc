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
	progNormalMap{GLProgram::createFromFile("normalMapVertex.glsl", "normalMapFragment.glsl")},
	mvpLocationNormalMap{progNormalMap.getUniformLocation("MVP")},
	mLocationNormalMap{progNormalMap.getUniformLocation("M")},
	mitLocationNormalMap{progNormalMap.getUniformLocation("Mit")},
	invVLocationNormalMap{progNormalMap.getUniformLocation("invV")},
	lpLocationNormalMap{progNormalMap.getUniformLocation("vLightPos")},
	texRescaleLocationNormalMap{progNormalMap.getUniformLocation("texRescale")},
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
	progNormalMap.setTexture(normMapLocationNormalMap,brickNormalMap,0);
	progNormalMap.setTexture(texLocationNormalMap,brickAlbedo,1);
	progNormalMap.setUniform(opacityLocation, 1.0f);
	
	brickArray.bind();

	uint32_t i{0};
	for (uint32_t y = 0;y < height();++y) {
		for (uint32_t x = 0;x < width();++x) {
			const Vec3 c = colorData[i++];
			
			if (c.r() < 0) continue; // empty spaces
			
			const Mat4 m{Mat4::translation({float(x)-float(width())/2,float(height()-y)-float(height()/2), -20.0f})};
			progNormalMap.setUniform(texRescaleLocationNormalMap, 1.0f);
			progNormalMap.setUniform(mvpLocationNormalMap, m*v*p);
			progNormalMap.setUniform(mLocationNormalMap, m);
			progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(m), true);
			progNormalMap.setUniform(invVLocationNormalMap, Mat4::inverse(v));
			progNormalMap.setUniform(colorLocation, c);
			glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
		}
	}
	
	
	for (const Vec2i& pos : tetrominoPos) {
		const Mat4 m{Mat4::translation({float(pos.x())-float(width())/2,float(height()-pos.y())-float(height()/2), -20.0f})};
		progNormalMap.setUniform(texRescaleLocationNormalMap, 1.0f);
		progNormalMap.setUniform(mvpLocationNormalMap, m*v*p);
		progNormalMap.setUniform(mLocationNormalMap, m);
		progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(m), true);
		progNormalMap.setUniform(invVLocationNormalMap, Mat4::inverse(v));
		progNormalMap.setUniform(colorLocation, currentColor);
		glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
	}	

	for (const Vec2i& pos : nextTetrominoPos) {
		const Mat4 m{Mat4::translation({float(pos.x())-float(width())/2+13,float(height()-pos.y())-float(height()/2)-17, -20.0f})};
		progNormalMap.setUniform(texRescaleLocationNormalMap, 1.0f);
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
		const Mat4 m{Mat4::translation({float(pos.x())-float(width())/2,float(height()-pos.y())-float(height()/2), -20.0f})};
		progNormalMap.setUniform(texRescaleLocationNormalMap, 1.0f);
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