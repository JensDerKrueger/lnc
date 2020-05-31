from abc import ABC, abstractmethod
from vec3 import vec3

class hitRecord:
	def __init__(self, t, p, n, r, mat):
		self.t = t
		self.p = p
		self.r = r
		self.mat = mat
		
		self.frontFace = vec3.dot(r.direction(), n) < 0
		self.n = n if self.frontFace else n*-1
		

class hitable:
	@abstractmethod	
	def hit(self, r):
		pass