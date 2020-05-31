from abc import ABC, abstractmethod

class Renderer(ABC):
	def __init__(self, width, height):
		self.width = width
		self.height = height

	def __gridIndex(self,x,y):
		return x+self.width*y

	@abstractmethod
	def render(self, data):
		pass
