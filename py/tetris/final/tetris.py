import random, copy, time
import pygame
from TetrisConst import *

class Grid():
	def __init__(self, width, height, renderer):
		self.width = width
		self.height = height
		self.renderer = renderer
		self.clear()

	def clear(self):
		self.data = [-1] * self.width * self.height

	def setPixel(self,x,y,val):
		self.data[self.__gridIndex(x,y)] = val

	def getPixel(self,x,y):
		return self.data[self.__gridIndex(x,y)]

	def __gridIndex(self,x,y):
		return x+self.width*y

	def render(self, spritePixels):
		colorData = copy.deepcopy(self.data)
		for i in range(len(colorData)):
			colorData[i] = colors[colorData[i]]
		for p in spritePixels:
			index = self.__gridIndex(p[0][0], p[0][1])
			colorData[index] = [x[0]*p[2]+x[1]*(1-p[2]) for x in zip(p[1], colorData[index])]
		self.renderer.render(colorData)

	def __str__(self):
		r = ""
		for i in range(self.height):
			r += str(self.data[i*self.width:(i+1)*self.width])+"\n"
		return r

class Tetris:
	def __init__(self, grid):
		self.grid = grid
		self.__startGame()

	def __startGame(self, level=1):
		self.grid.clear()
		self.totalRows = 0
		self.current = self.__genRandTetrominoIndex()
		self.next = self.__genRandTetrominoIndex()
		self.rotation = 0
		self.position = [self.grid.width//2,0]
		self.score = 0
		self.level = 0
		self.clearedRows = 0

	def rotateCW(self):
		nextRotation = (self.rotation+1)%4
		if self.__validateTransform(nextRotation, self.position):
			self.rotation = nextRotation

	def rotateCCW(self):
		nextRotation = (self.rotation+3)%4
		if self.__validateTransform(nextRotation, self.position):
			self.rotation = nextRotation

	def moveLeft(self):
		nextPosition = [self.position[0]-1,self.position[1]]
		if self.__validateTransform(self.rotation, nextPosition):
			self.position = nextPosition

	def moveRight(self):
		nextPosition = [self.position[0]+1,self.position[1]]
		if self.__validateTransform(self.rotation, nextPosition):
			self.position = nextPosition

	def fullDrop(self):
		nextPosition = [self.position[0],self.position[1]+1]
		while self.__validateTransform(self.rotation, nextPosition):
			self.position = nextPosition
			nextPosition = [self.position[0],self.position[1]+1]

	def advance(self):
		nextPosition = [self.position[0],self.position[1]+1]
		if not self.__validateTransform(self.rotation, nextPosition):
			self.__applyCollision()
			fullRows = self.__checkRows()
			self.__updateScore(len(fullRows))
			for i in fullRows:
				self.__clearRow(i)
			self.current = self.next
			self.next = self.__genRandTetrominoIndex()
			self.rotation = 0
			self.position = [self.grid.width//2,0]
			if not self.__validateTransform(self.rotation, self.position):
				return False
		else:
			self.position = nextPosition
		return True

	def render(self):
		pixeldata = []
		
		# create sprite for current tetromino
		tetrominoPos = self.__computeGridPositions(self.current, self.rotation , self.position)
		for p in tetrominoPos:
			pixeldata += [[p,colors[self.current],1]]
		
		# create sprite for next tetromino preview
		tetrominoPos = self.__computeGridPositions(self.next, 0, [2,0])
		for p in tetrominoPos:
			pixeldata += [[p,colors[self.next],0.2]]
		print(pixeldata)
		self.grid.render(pixeldata)

	@staticmethod
	def __genRandTetrominoIndex():
		return random.randint(0,len(tetrominos)-1)

	def __checkRows(self):
		fullRows = []
		for y in range(0,self.grid.height):
			for x in range(0,self.grid.width):
				if self.grid.getPixel(x, y) < 0: break
			else:
				fullRows.append(y)
		return fullRows

	def __validateTransform(self, rot, pos):
		gridPos = self.__computeGridPositions(self.current,rot, pos)
		for p in gridPos:
			if p[1] >= self.grid.height: return False
			if p[0] >= self.grid.width: return False
			if p[0] < 0: return False
			if self.grid.getPixel(p[0],p[1]) >= 0:
				return False
		return True

	def __clearRow(self, row):
		self.clearedRows += 1
		if self.clearedRows % 10 == 0:
			self.level += 1
		for y in range(row,0,-1):
			for x in range(0,self.grid.width):
				self.grid.setPixel(x,y,self.grid.getPixel(x,y-1))
		for x in range(0,self.grid.width):
				self.grid.setPixel(x,0,-1)

	def __applyCollision(self):
		gridPos = self.__computeGridPositions(self.current, self.rotation , self.position)
		for p in gridPos:
			self.grid.setPixel(p[0], p[1], self.current)

	def __computeGridPositions(self, t, rot, pos):
		gridPos = copy.deepcopy(tetrominos[t][rot%len(tetrominos[t])])
		for i in range(len(gridPos)):
			gridPos[i][0] += pos[0]
			gridPos[i][1] += pos[1]
			if gridPos[i][1] < 0:  # if the brick collides with the top, lower it once
				pos[1] += 1
				gridPos = self.__computeGridPositions(t, rot, pos)
				break
		return gridPos

	def __getLevel(self):
		return self.totalRows//10

	def getDelay(self):
		level = self.__getLevel()
		if level == 0:
			delay = 48
		elif level <= 1:
			delay = 43
		elif level <= 2:
			delay = 38
		elif level <= 3:
			delay = 33
		elif level <= 4:
			delay = 28
		elif level <= 5:
			delay = 23
		elif level <= 6:
			delay = 18
		elif level <= 7:
			delay = 13
		elif level <= 8:
			delay = 8
		elif level <= 9:
			delay = 6
		elif level <= 12:
			delay = 5
		elif level <= 15:
			delay = 4
		elif level <= 18:
			delay = 3
		elif level <= 28:
			delay = 2
		else:
			delay = 1

		return 8*delay

	def __updateScore(self, rowCount):
		if rowCount == 1:
			points = 40
		elif rowCount == 2:
			points = 100
		elif rowCount == 3:
			points = 300
		else:
			points = 1200
		self.score += (self.__getLevel()+1)*points
		self.totalRows += rowCount

try:
	from LedRenderer import LedRenderer
	r = LedRenderer(15,20)
except ImportError:
	from TextRenderer import TextRenderer
	r = TextRenderer(15,20)

g = Grid(r.width,r.height,r)
t = Tetris(g)

pygame.init()
pygame.display.set_mode((1, 1))
pygame.joystick.init()
joysticks = [pygame.joystick.Joystick(x) for x in range(pygame.joystick.get_count())]
for j in joysticks:
	j.init()

ok = True
while ok:
	t.render()

	step = 0
	while step < t.getDelay():
		event = pygame.event.poll()

		if event.type == pygame.JOYAXISMOTION:
			if event.axis == 0:
				if event.value <= -0.2:
					t.moveLeft()
					t.render()
				elif event.value >= 0.2:
					t.moveRight()
					t.render()
			if event.axis == 1:
				if event.value <= -0.2:
					t.fullDrop()
					t.render()
					break
				elif event.value >= 0.2:
					break # drop one line
		if event.type == pygame.JOYBUTTONDOWN:
			if event.button == 1:   # button A - rotate CCW
				t.rotateCCW()
				t.render()
			elif event.button == 0:   # button B - rotate CW
				t.rotateCW()
				t.render()

		if event.type == pygame.KEYDOWN:
			if event.key == pygame.K_LEFT:
				t.moveLeft()
				t.render()
			elif event.key == pygame.K_RIGHT:
				t.moveRight()
				t.render()
			elif event.key == pygame.K_w:
				t.rotateCW()
				t.render()
			elif event.key == pygame.K_q:
				t.rotateCCW()
				t.render()
			elif event.key == pygame.K_DOWN:
				break
			elif event.key == pygame.K_SPACE:
				t.fullDrop()
				t.render()
				break

		time.sleep(0.01)
		step += 1

	ok = t.advance()
