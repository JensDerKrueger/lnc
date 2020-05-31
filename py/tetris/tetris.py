from TetrisConst import colors, tetrominos
from TextRenderer import TextRenderer
import pygame, time, random, copy

class Grid:
	def __init__(self, width, height, renderer):
		self.width = width
		self.height = height
		self.renderer = renderer
		self.clear()
		
	def clear(self):
		self.data = self.width * self.height * [-1]
	
	def __serializeIndex(self, x, y):
		return x+self.width*y
	
	def setPixel(self, x, y, val):
		self.data[self.__serializeIndex(x,y)] = val
		
	def getPixel(self, x, y):
		return self.data[self.__serializeIndex(x,y)]
		
	def render(self, spritePixel):
		color = [[0,0,0]] * len(self.data)
		
		for i in range(len(self.data)):
			color[i] = colors[self.data[i]]
	
		for p in spritePixel:
			index = self.__serializeIndex(p[0],p[1])					
			color[index] = [c[0] * (1-p[3]) + c[1] * p[3] for c in zip(color[index],colors[p[2]])]
						
		self.renderer.render(color)
		
	def __str__(self):
		result = "Width="+str(self.width)+" Height="+str(self.height)+"\n"
		
		for i in range(len(self.data)):
			if i > 0 and i%self.width == 0:
				result += "\n"
			result += str(self.data[i])+"\t"
		
		return result
		

class Tetris:
	def __init__(self, grid):
		self.grid = grid
		self.__startGame()
		
	def __startGame(self):
		self.totalRows = 0
		self.__resetTransformation()
		self.current = self.__genRandomTetromino()
		self.next = self.__genRandomTetromino()
		self.score = 0
		
	def __resetTransformation(self):
		self.rotation = 0
		self.position = [self.grid.width//2,0]		
				
	def rotate180(self):
		rotation = (self.rotation+2)%4
		if self.__validateTransform(rotation, self.position):
			self.rotation = rotation
	
	def rotateCW(self):
		rotation = (self.rotation+1)%4
		if self.__validateTransform(rotation, self.position):
			self.rotation = rotation
		
	def rotateCCW(self):
		rotation = (self.rotation+3)%4
		if self.__validateTransform(rotation, self.position):
			self.rotation = rotation
		
	def moveLeft(self):
		position = [self.position[0]-1, self.position[1]]
		if self.__validateTransform(self.rotation, position):
			self.position = position
		
	def moveRight(self):
		position = [self.position[0]+1, self.position[1]]
		if self.__validateTransform(self.rotation, position):
			self.position = position
		
	def fullDrop(self):
		position = [self.position[0], self.position[1]+1]
		while self.__validateTransform(self.rotation, position):
			self.position = position
			position = [self.position[0],self.position[1]+1]
		
	def render(self):
		
		terominoPos = self.__computeGridPosition(self.current, self.rotation, self.position)			
		spritePixel = [[p[0],p[1],self.current,1] for p in terominoPos]

		terominoPos = self.__computeGridPosition(self.next, 0, [2,0])			
		spritePixel += [[p[0],p[1],self.next,0.3] for p in terominoPos]
		
		self.grid.render(spritePixel)
				
	def advance(self):
		position = [self.position[0], self.position[1]+1]
		if not self.__validateTransform(self.rotation, position):
			self.__applyCollision()
			fullRows = self.__checkRows()
			for row in fullRows:
				self.__clearRow(row)
			self.current = self.next
			self.next = self.__genRandomTetromino()
			self.__resetTransformation()
			self.__updateScore(len(fullRows))
			if not self.__validateTransform(self.rotation, self.position):
				return False
		else:
			self.position = position
		return True
		
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
		
		
	def __clearRow(self, row):
		for y in range(row, 0, -1):
			for x in range(0,self.grid.width):
				self.grid.setPixel(x,y,self.grid.getPixel(x,y-1))				
		for x in range(0,self.grid.width):
			self.grid.setPixel(x,0,-1)
		
	
	@staticmethod
	def __genRandomTetromino():
		return random.randint(0,len(tetrominos)-1)
		
	def __applyCollision(self):
		gridPos = self.__computeGridPosition(self.current, self.rotation, self.position)
		for p in gridPos:
			self.grid.setPixel(p[0], p[1], self.current)
			
	def __computeGridPosition(self, tIndex, rot, pos):
		gridPos = copy.deepcopy(tetrominos[tIndex][rot%len(tetrominos[tIndex])])
		for i in range(len(gridPos)):
			gridPos[i][0] += pos[0]
			gridPos[i][1] += pos[1]
			
			if gridPos[i][1] < 0:
				pos[1] += 1
				gridPos = self.__computeGridPosition(tIndex, rot, pos)
				break
				
		return gridPos
		
	def __checkRows(self):
		fullRows = []
		for y in range(0, self.grid.height):
			for x in range(0,self.grid.width):
				if self.grid.getPixel(x,y) == -1: break
			else:
				fullRows += [y]
		return fullRows

	def __validateTransform(self, rot, pos):
		gridPos = self.__computeGridPosition(self.current, rot, pos)
		for p in gridPos:
			if p[1] >= self.grid.height: return False
			if p[0] >= self.grid.width: return False
			if p[0] < 0: return False
			if self.grid.getPixel(p[0],p[1]) >= 0:
				return False
		return True
	
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
		
		

r = TextRenderer(10,20)
g = Grid(r.width,r.height,r)
t = Tetris(g)

pygame.init()
pygame.display.set_mode((1,1))

ok = True
while ok:
	t.render()
	
	delay = 0
	while delay < t.getDelay():
		event = pygame.event.poll()
		
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
			elif event.key == pygame.K_e:
				t.rotate180()
				t.render() 
			elif event.key == pygame.K_DOWN:
				break
			elif event.key == pygame.K_SPACE:
				t.fullDrop()
				t.render()
				break
				
		time.sleep(0.01)
		delay += 1
		
	ok = t.advance()
	
	