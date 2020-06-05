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
	colorLocation{progNormalMap.getUniformLocation("color")}
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
							const std::vector<Vec3>& colorData) {
								
	// setup basic OpenGL states that do not change during the frame
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);    
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	const Vec3 lookFromVec{Vec3{0,0,-5}};
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
	
	brickArray.bind();

	uint32_t i{0};
	for (uint32_t y = 0;y < height();++y) {
		for (uint32_t x = 0;x < width();++x) {
			const Vec3 c = colorData[i++];
			
			if (c.r() < 0) continue; // empty spaces
			
			const Mat4 m{Mat4::translation({float(width()-x)-float(width())/2,float(height()-y)-float(height()/2), 20.0f})};
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
		const Mat4 m{Mat4::translation({float(width()-pos.x())-float(width())/2,float(height()-pos.y())-float(height()/2), 20.0f})};
		progNormalMap.setUniform(texRescaleLocationNormalMap, 1.0f);
		progNormalMap.setUniform(mvpLocationNormalMap, m*v*p);
		progNormalMap.setUniform(mLocationNormalMap, m);
		progNormalMap.setUniform(mitLocationNormalMap, Mat4::inverse(m), true);
		progNormalMap.setUniform(invVLocationNormalMap, Mat4::inverse(v));
		progNormalMap.setUniform(colorLocation, currentColor);
		glDrawElements(GL_TRIANGLES, brick.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
	}	
	
							
}
							
/*

struct SpritePixel {
	SpritePixel(Vec2i pos, const Vec3& rgb, float alpha) :
		pos{pos},
		rgb(rgb),
		alpha(alpha)
	{}
	
	Vec2i pos;
	Vec3 rgb;
	float alpha;
};

void TextRenderer::render(const std::array<Vec2i,4>& tetrominoPos, const Vec3& currentColor,
						  const std::array<Vec2i,4>& nextTetrominoPos, const Vec3& nextColor,
						  const std::vector<Vec3>& colorDataIn) {
							
	std::vector<Vec3> colorData{colorDataIn};
	
	std::vector<SpritePixel> spritePixels;
	for (const Vec2i& p : tetrominoPos) {
		spritePixels.push_back(SpritePixel{p,currentColor,1});
	}	
	for (const Vec2i& p : nextTetrominoPos) {
		spritePixels.push_back(SpritePixel{p,nextColor,0.2});
	}	

	for (const auto& p : spritePixels) {
		if (p.pos.y() < 0 || p.pos.x() < 0 || p.pos.y() >= height() || p.pos.x() >= width()) continue;
		size_t index = gridIndex(p.pos.x(), p.pos.y());
		colorData[index] = colorData.at(index) * (1-p.alpha) + p.rgb * p.alpha;
	}
	
	
	std::cout << "\033[2J\033[;H";
	uint32_t i{0};
	for (uint32_t y = 0;y < height();++y) {
		for (uint32_t x = 0;x < width();++x) {
			setColor(colorData[i++]);
			std::cout << "  ";
		}
		setColor(Vec3{0,0,0});
		std::cout << "\n";
	}
	std::cout << std::flush;
}

void TextRenderer::setColor(const Vec3& color) {
	const uint32_t index = 16 + uint32_t(color.r()*5) + 
							6 * uint32_t(color.g()*5) + 
						   36 * uint32_t(color.b()*5);
							
	std::cout << "\033[48;5;" << index << "m";
}

*/