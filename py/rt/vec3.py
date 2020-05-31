import math
import random

class vec3:
	def __init__(self, e1, e2, e3):
		self.e = [e1,e2,e3]
		
	def __str__(self):
		return repr(self.e)
		
	def x(self):
		return self.e[0]
		
	def y(self):
		return self.e[1]
		
	def z(self):
		return self.e[2]			
		
	def r(self):
		return self.e[0]
		
	def g(self):
		return self.e[1]
		
	def b(self):
		return self.e[2]	
		
	def __add__(self, other):
		return vec3(self.e[0]+other.e[0], self.e[1]+other.e[1], self.e[2]+other.e[2])

	def __sub__(self, other):
		return vec3(self.e[0]-other.e[0], self.e[1]-other.e[1], self.e[2]-other.e[2])

	def __mul__(self, other):
		if type(other) == vec3:
			return vec3(self.e[0]*other.e[0], self.e[1]*other.e[1], self.e[2]*other.e[2])
		else:
			return vec3(self.e[0]*other, self.e[1]*other, self.e[2]*other)
		
	def __truediv__(self, other):
		if type(other) == vec3:
			return vec3(self.e[0]/other.e[0], self.e[1]/other.e[1], self.e[2]/other.e[2])
		else:
			return vec3(self.e[0]/other, self.e[1]/other, self.e[2]/other)
		
	def length(self):
		return math.sqrt(self.squareLength())

	def squareLength(self):
		return self.e[0]*self.e[0]+self.e[1]*self.e[1]+self.e[2]*self.e[2]

	@staticmethod
	def dot(a, b):
		return a.e[0]*b.e[0] + a.e[1]*b.e[1] + a.e[2]*b.e[2]

	@staticmethod
	def cross(a, b):
		return vec3(a.e[1] * b.e[2] - a.e[2] * b.e[1],
					a.e[2] * b.e[0] - a.e[0] * b.e[2],
					a.e[0] * b.e[1] - a.e[1] * b.e[0])

	@staticmethod
	def normalize(v):
		return v/v.length()

	@staticmethod
	def reflect(v, n):
		return v-n*vec3.dot(v,n)*2
		
	@staticmethod
	def refract(uv, n, etaiOverEtat):
		cosTheta            = min(-vec3.dot(uv, n), 1.0)
		rOutParallel        = (uv + n*cosTheta) * etaiOverEtat
		rOutPerpendicular   = n * -math.sqrt(1.0 - min(rOutParallel.squareLength(),1))
		return rOutParallel + rOutPerpendicular

	@classmethod				
	def random(cls):
		return cls(random.uniform(0,1),random.uniform(0,1),random.uniform(0,1))		
	
	@classmethod				
	def randomPointInSphere(cls):
		while True:
			p = cls(random.uniform(-1,1),random.uniform(-1,1),random.uniform(-1,1))		
			if p.squareLength() > 1: continue
			return p

	@classmethod				
	def randomPointInDisc(cls):
		while True:
			p = cls(random.uniform(-1,1),random.uniform(-1,1),0)
			if p.squareLength() > 1: continue
			return p

					
if __name__ == "__main__":
	x = vec3(1,2,2)
	y = vec3(1,1,2)
	print(vec3.cross(x,y))
