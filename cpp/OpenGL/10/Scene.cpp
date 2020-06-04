#include <iostream>

#include "Renderer.h"

#include "Scene.h"
#include "TetrisConst.h"

Scene::Scene(Grid& grid) : 
	grid{grid},
	current{genRandTetrominoIndex()},
	next{genRandTetrominoIndex()},
	rotationIndex{0},
	position{int32_t(grid.getWidth())/2,0},
	score{0},
	level{0},
	clearedRows{0}
{
	grid.clear();
}


void Scene::rotateCW(){
	size_t nextRotationIndex = (rotationIndex+1)%4;
	if (validateTransform(nextRotationIndex,position)) {
		rotationIndex = nextRotationIndex;
	}
}

void Scene::rotateCCW(){
	size_t nextRotationIndex = (rotationIndex+3)%4;
	if (validateTransform(nextRotationIndex,position)) {
		rotationIndex = nextRotationIndex;
	}	
}

void Scene::moveLeft(){
	Vec2i nextPosition{position.x()-1, position.y()};
	if (validateTransform(rotationIndex,nextPosition)) {
		position = nextPosition;
	}		
}

void Scene::moveRight(){
	Vec2i nextPosition{position.x()+1, position.y()};
	if (validateTransform(rotationIndex,nextPosition)) {
		position = nextPosition;
	}	
}

bool Scene::advance() {
	Vec2i nextPosition{position.x()+1, position.y()};
	
	if (validateTransform(rotationIndex,nextPosition)) {
		applyCollision();
		const std::vector<uint32_t> fullRows = checkRows();
		updateScore(fullRows.size());
		for (uint32_t row : fullRows) {
			clearRow(row);
		}
		current = next;
		next = genRandTetrominoIndex();
		rotationIndex = 0;
		position = Vec2i{int32_t(grid.getWidth())/2,0};
		if (validateTransform(rotationIndex,position)) 
			return false;
		
	} else {
		position = nextPosition;
	}
	return true;
}

void Scene::fullDrop() {
	Vec2i nextPosition{position.x(), position.y()+1};
	while (validateTransform(rotationIndex,nextPosition)) {
		position = nextPosition;
		nextPosition = Vec2i{position.x(), position.y()+1};
	}	
}


void Scene::render() {
	std::vector<SpritePixel> spritePixels;
	std::array<Vec2i,4> tetrominoPos{computeGridPositions(current, rotationIndex, position)};
	for (const Vec2i& p : tetrominoPos) {
		spritePixels.push_back(SpritePixel{p,colors[current],1});
	}

	tetrominoPos = computeGridPositions(next, 0, Vec2i{2,0});
	for (const Vec2i& p : tetrominoPos) {
		spritePixels.push_back(SpritePixel{p,colors[current],0.2});
	}
	grid.render(spritePixels);
}


size_t Scene::genRandTetrominoIndex() {
	return 0; // TODO
}

std::vector<uint32_t> Scene::checkRows() const {
	return {}; // TODO
}

bool Scene::validateTransform(uint32_t rot, const Vec2i& pos) const {
	return true; // TODO
}

void Scene::clearRow(uint32_t r) {
	// TODO
}

void Scene::applyCollision() {
	// TODO
}

std::array<Vec2i,4> Scene::computeGridPositions(size_t tet, uint32_t rot, const Vec2i& pos) const {
	return {}; // TODO	
}

uint32_t Scene::getLevel() const {
	return 0; // TODO
}

uint32_t Scene::getDelay() const {
	return 0; // TODO
}

void Scene::updateScore(size_t rowCount) {
	// TODO
}