#include <iostream>
#include <fstream>
#include <algorithm>

#include <Rand.h>

#include "Renderer.h"

#include "Scene.h"
#include "TetrisConst.h"

Scene::Scene(Grid& grid) : 
	grid{grid},
	current{0},
	next{0},
	rotationIndex{0},
	position{int32_t(grid.getWidth())/2,0},
	prevPosition{position},
	score{0},
	clearedRows{0},
	lastAdvance{-1},
  pause(false),
  gameOver(false),
  chatConnection("134.91.11.186", 11006, this)
{
	current = genRandTetrominoIndex();
	next = genRandTetrominoIndex();
	grid.clear();
  
  loadHighscore();

  std::sort(highscore.begin(), highscore.end(),[](const std::pair<std::string, uint32_t> &x,
                                                  const std::pair<std::string, uint32_t> &y) {return x.second > y.second;});
  grid.getRenderer()->updateHighscore(highscore);
}

void Scene::restart() {
  score = 0;
  clearedRows = 0;
  current = genRandTetrominoIndex();
  next = genRandTetrominoIndex();
  grid.clear();
  gameOver = false;
  gameOverTime = 0;
  grid.getRenderer()->setGameOver(false, score);
}

void Scene::rotateCW(){
    if (pause || gameOver) return;
	size_t nextRotationIndex = (rotationIndex+1)%4;
	if (validateTransform(nextRotationIndex,position)) {
		rotationIndex = nextRotationIndex;
	}
}

void Scene::rotateCCW(){
    if (pause || gameOver) return;
	size_t nextRotationIndex = (rotationIndex+3)%4;
	if (validateTransform(nextRotationIndex,position)) {
		rotationIndex = nextRotationIndex;
	}	
}

void Scene::moveLeft(){
    if (pause || gameOver) return;
	prevPosition = position;
	Vec2i nextPosition{position.x()-1, position.y()};
	if (validateTransform(rotationIndex,nextPosition)) {
		position = nextPosition;
	}		
}

void Scene::moveRight(){
    if (pause || gameOver) return;
	prevPosition = position;
	Vec2i nextPosition{position.x()+1, position.y()};
	if (validateTransform(rotationIndex,nextPosition)) {
		position = nextPosition;
	}	
}

bool Scene::evaluateState(const Vec2i& nextPosition) {
	if (!validateTransform(rotationIndex,nextPosition)) {
		applyCollision();
		const std::vector<uint32_t> fullRows = checkRows();
		if (!fullRows.empty()) {
			updateScore(uint32_t(fullRows.size()));
			for (uint32_t row : fullRows) {
				clearRow(row);
			}
		}
		grid.getRenderer()->dropAnimation(transformTetromino(current, rotationIndex, prevPosition), colors[current],
										  transformTetromino(current, rotationIndex, position), fullRows);
		current = next;
		next = genRandTetrominoIndex();
		rotationIndex = 0;
		position = Vec2i{int32_t(grid.getWidth())/2,0};
		if (!validateTransform(rotationIndex,position)) return false;	
	} else {
		position = nextPosition;
	}
	return true;	
}

bool Scene::advance() {
	prevPosition = position;
	Vec2i nextPosition{position.x(), position.y()+1};
	return evaluateState(nextPosition);
}

Vec2i Scene::fullDropPosition() const {
	Vec2i fullDropPosition{position.x(), position.y()+1};
	while (validateTransform(rotationIndex,fullDropPosition)) {
		fullDropPosition = Vec2i{fullDropPosition.x(), fullDropPosition.y()+1};
	}
	return Vec2i{fullDropPosition.x(), fullDropPosition.y()-1};
}

void Scene::fullDrop() {
  if (pause || gameOver) return;
	prevPosition = position;
	position = fullDropPosition();
	if (!evaluateState(position+Vec2i(0,1))) setGameOver();
}

void Scene::setGameOver() {
  gameOver = true;
  std::cout << "Score: " << score << " at level " << getLevel() << std::endl;
  grid.getRenderer()->setGameOver(true, score);
}

bool Scene::render(double t) {
	if (lastAdvance < 0) lastAdvance = t;
	
	std::array<Vec2i,4> transformedCurrent{transformTetromino(current, rotationIndex, position)};
	std::array<Vec2i,4> transformedNext{transformTetromino(next, 0, Vec2i{0,0})};
	std::array<Vec2i,4> transformedTarget{transformTetromino(current, rotationIndex, fullDropPosition())};

  if (!pause) {
    if (getDelay() * 0.01 < t-lastAdvance && !grid.getRenderer()->isAnimating() && !gameOver) {
      if (!advance()) {
        setGameOver();
        return false;
      }
      lastAdvance = t;
    }
  }

  renderMutex.lock();
  grid.render(transformedCurrent, colors[current], transformedNext,
              colors[next], transformedTarget, float(pause ? lastAdvance : t));
  renderMutex.unlock();

  if (gameOver && gameOverTime == 0)
    gameOverTime = t;
  if (gameOver && t-gameOverTime > 10) {
    restart();
  }
  
  return true;
}

size_t Scene::genRandTetrominoIndex() {
	if (nextTetrominos.empty()) {
		for (size_t i = 0;i<tetrominos.size()*3;++i) {
			nextTetrominos.push_back(i%tetrominos.size());
		}
		Rand::shuffle(nextTetrominos);
	}	
	size_t v = nextTetrominos.back();
	nextTetrominos.pop_back();
	return v;
}

std::vector<uint32_t> Scene::checkRows() const {
	std::vector<uint32_t> fullRows;
	for (uint32_t y = 0;y<grid.getHeight();++y) {
		bool isFullRow = true;
		for (uint32_t x = 0;x<grid.getWidth();++x) {
			if (grid.getPixel(x, y) < 0) {
				isFullRow = false;
				break;
			}
		}
		if (isFullRow) fullRows.push_back(y);
	}
	return fullRows;
}

bool Scene::validateTransform(size_t rot, const Vec2i& pos) const {
	std::array<Vec2i,4> transformedTetromino = transformTetromino(current, rot, pos);
	for (const Vec2i& brick : transformedTetromino) {
		if (brick.x() < 0) return false;
		if (brick.y() >= int64_t(grid.getHeight())) return false;
		if (brick.x() >= int64_t(grid.getWidth())) return false;
		if (grid.getPixel(uint32_t(brick.x()), uint32_t(brick.y())) >= 0) return false;
	}
	return true;
}

void Scene::clearRow(uint32_t row) {
	clearedRows += 1;
	for (uint32_t y = row;y>0;--y) {
		for (uint32_t x = 0;x<grid.getWidth();++x) {
			grid.setPixel(x,y,grid.getPixel(x,y-1));
		}
	}
	for (uint32_t x = 0;x<grid.getWidth();++x) {
		grid.setPixel(x,0,-1);
	}	
}

void Scene::applyCollision() {
	std::array<Vec2i,4> transformedTetromino = transformTetromino(current, rotationIndex, position);
	for (const Vec2i& brick : transformedTetromino)
		grid.setPixel(uint32_t(brick.x()), uint32_t(brick.y()), int8_t(current));	
}

std::array<Vec2i,4> Scene::transformTetromino(size_t tetIndex, size_t rot, const Vec2i& pos) const {
	std::array<Vec2i,4> transformedTetromino = tetrominos[tetIndex][rot%tetrominos[tetIndex].size()];
	for (uint32_t i = 0;i<transformedTetromino.size();++i) {
		transformedTetromino[i] = transformedTetromino[i] + pos;
		if (transformedTetromino[i].y() < 0) { // when the tetromino collides with the top of the grid then try to lower it by one
			transformedTetromino = transformTetromino(tetIndex, rot, pos+Vec2i{0,1});
			break;
		}
	}
	return transformedTetromino;
}

uint32_t Scene::getLevel() const {
	return clearedRows/10;
}

uint32_t Scene::getDelay() const {
	const uint32_t scale = 20;
	switch (getLevel()) {
		case 0 : return scale*48;
		case 1 : return scale*43;
		case 2 : return scale*38;
		case 3 : return scale*33;
		case 4 : return scale*28;
		case 5 : return scale*23;
		case 6 : return scale*18;
		case 7 : return scale*13;
		case 8 : return scale* 8;
		case 9 :
		case 10:
		case 11: return scale* 6;
		case 12: return scale* 5;
		case 13:
		case 14:
		case 15: return scale* 4;
		case 16:
		case 17:
		case 18: return scale* 3;
		default: return scale* 2;
	}
}

size_t Scene::getPlayerIndex(const std::string& name) {
  for (size_t i = 0;i<highscore.size();++i) {
    if (highscore[i].first == name) return i;
  }
  highscore.push_back({name,0});
  return highscore.size()-1;
}

void Scene::updateScore(uint32_t rowCount) {
	uint32_t points{0};
	switch (rowCount) {
		case 1 : points = 40; break;
		case 2 : points = 100; break;
		case 3 : points = 300; break;
		case 4 : points = 1200; break;
	}
	score += (getLevel()+1)*points;
	clearedRows += rowCount;
  
  if (activePlayer != "" && activePlayer != "bitmatcher")   // exclude debugging from highscore
    highscore[getPlayerIndex(activePlayer)].second += (getLevel()+1)*points;
  
  std::sort(highscore.begin(), highscore.end(),[](const std::pair<std::string, uint32_t> &x,
                                                  const std::pair<std::string, uint32_t> &y) {return x.second > y.second;});
  grid.getRenderer()->updateHighscore(highscore);
  saveHighscore();
}


void Scene::saveHighscore() {
  std::ofstream scoreFile ("scores.data");
  if (scoreFile.is_open()) {
    for (const auto& e : highscore) {
      scoreFile << base64_encode(e.first) << ";" << e.second << ";"<< std::endl;
    }
    scoreFile.close();
  }
  std::ofstream csvScoreFile ("scores.csv");
  if (csvScoreFile.is_open()) {
    csvScoreFile << "Place;Name;Score" << std::endl;
    size_t i = 0;
    for (const auto& e : highscore) {
      if (e.second > 0)
        csvScoreFile << i+1 << ";" << e.first << ";" << e.second << std::endl;
    }
    csvScoreFile.close();
  }
}

void Scene::loadHighscore() {
  std::string line;
  highscore.clear();
  std::ifstream scoreFile("scores.data");
  try {
    if (scoreFile.is_open()) {
      while (getline(scoreFile,line) && !line.empty() ) {
        Tokenizer tokenizer{line, ';'};
        const std::string encName = tokenizer.nextString();
        const std::string name = base64_decode(encName);
        const uint32_t playerScore = tokenizer.nextUint32();
        highscore.push_back({name,playerScore});
      }
      scoreFile.close();
    }
  } catch (const MessageException& ) {
  }
}

void Scene::setActivePlayer(const std::string& name) {
  getPlayerIndex(name);
  activePlayer = name;
}


ChatConnection::ChatConnection(const std::string& address, short port, Scene* app) :
Client{address, port , "", 5000},
app{app}
{
}

bool ChatConnection::executeCommand(char c, const std::string& player) {
  const std::scoped_lock<std::mutex> lock(app->renderMutex);
  switch (c) {
    case 'w':
      app->setActivePlayer(player);
      app->fullDrop();
      break;
    case 's':
      app->setActivePlayer(player);
      app->advance();
      break;
    case 'a':
      app->setActivePlayer(player);
      app->moveLeft();
      break;
    case 'd':
      app->setActivePlayer(player);
      app->moveRight();
      break;
    case 'q':
      app->setActivePlayer(player);
      app->rotateCCW();
      break;
    case 'e':
      app->setActivePlayer(player);
      app->rotateCW();
      break;
    default:
      return false;
      break;
  }
  return true;
}

void ChatConnection::handleServerMessage(const std::string& message) {
  try {
    Tokenizer t{message, char(1)};
    const std::string player  = base64url_decode(t.nextString());
    const std::string command = base64url_decode(t.nextString());
        
    for (char c : command) {
      if (!executeCommand( c, player )) break;
    }
  } catch (const MessageException e) {
  }
}
