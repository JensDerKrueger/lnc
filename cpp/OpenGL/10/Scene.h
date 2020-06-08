#pragma once

#include <vector>

#include "TetrisConst.h"
#include "Grid.h"

class Scene {
public:
	Scene(Grid& grid);
	
	void rotateCW();
	void rotateCCW();
	void moveLeft();
	void moveRight();
	bool advance();
	void fullDrop();
	bool render(double t);
	
	void setShowPreview(bool showPreview) {grid.getRenderer()->setShowPreview(showPreview);}
	bool getShowPreview () const {return grid.getRenderer()->getShowPreview();}

	void setShowTarget(bool showTarget) {grid.getRenderer()->setShowTarget(showTarget);}
	bool getShowTarget () const {return grid.getRenderer()->getShowTarget();}
	
	
private:
	std::vector<size_t> nextTetrominos;
	Grid grid;
	size_t current;
	size_t next;
	size_t rotationIndex;
	Vec2i position;
	uint32_t score;
	uint32_t clearedRows;
	double lastAdvance;
	
	size_t genRandTetrominoIndex();
	std::vector<uint32_t> checkRows() const;
	bool validateTransform(uint32_t rot, const Vec2i& pos) const;
	void clearRow(uint32_t r);
	void applyCollision();
	std::array<Vec2i,4> transformTetromino(size_t tetIndex, uint32_t rot, const Vec2i& pos) const;
	uint32_t getLevel() const;
	uint32_t getDelay() const;
	void updateScore(size_t rowCount);
	Vec2i fullDropPosition() const;
		
};