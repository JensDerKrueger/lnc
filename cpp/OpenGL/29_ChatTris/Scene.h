#pragma once

#include <vector>
#include <mutex>

#include <Client.h>

#include "TetrisConst.h"
#include "Grid.h"

class Scene;

class ChatConnection : public Client {
public:
  ChatConnection(const std::string& address, uint16_t port, Scene* app);
  virtual void handleServerMessage(const std::string& message) override;

  void sendKeepAlivePing();
private:
  Scene* app;
  
  bool executeCommand(char c, const std::string& player);
};

class Scene {
public:
  std::mutex renderMutex;

	Scene(Grid& grid);
	
	void rotateCW();
	void rotateCCW();
	void moveLeft();
	void moveRight();
	bool advance();
	void fullDrop();
	bool render(double t);
	
	void setShowPreview(bool showPreview) {grid.getRenderer()->setShowPreview(showPreview);}
	bool getShowPreview() const {return grid.getRenderer()->getShowPreview();}

	void setShowTarget(bool showTarget) {grid.getRenderer()->setShowTarget(showTarget);}
	bool getShowTarget() const {return grid.getRenderer()->getShowTarget();}

  void setPause(bool pause) {this->pause = pause;}
  bool getPause() const {return pause;}

  void incBackgroundParam() {grid.getRenderer()->incBackgroundParam();}
  void decBackgroundParam() {grid.getRenderer()->decBackgroundParam();}
  
  void restart();
	
  void setActivePlayer(const std::string& name);
  
private:
	std::vector<size_t> nextTetrominos;
	Grid grid;
	size_t current;
	size_t next;
	size_t rotationIndex;
	Vec2i position;
	Vec2i prevPosition;
	uint32_t score;
	uint32_t clearedRows;
	double lastAdvance;
  bool pause;
  bool gameOver;
  double gameOverTime;
  ChatConnection chatConnection;
  std::string activePlayer;
  
	size_t genRandTetrominoIndex();
	std::vector<uint32_t> checkRows() const;
	bool validateTransform(size_t rot, const Vec2i& pos) const;
	void clearRow(uint32_t r);
	void applyCollision();
	std::array<Vec2i,4> transformTetromino(size_t tetIndex, size_t rot, const Vec2i& pos) const;
	uint32_t getLevel() const;
	uint32_t getDelay() const;
	void updateScore(uint32_t rowCount);
	Vec2i fullDropPosition() const;
	bool evaluateState(const Vec2i& nextPosition);
    
  void setGameOver();
  
  std::vector<std::pair<std::string, uint32_t>> highscore;
  size_t getPlayerIndex(const std::string& name);
  void loadHighscore();
  void saveHighscore();
};
