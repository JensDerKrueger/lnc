import math
import random

class Vec3 :
	def __init__(self, e0=0, e1=0, e2=0):
		self.e = [e0,e1,e2]

	def __str__(self):
		return "[" + str(self.e[0]) + ", " + str(self.e[1]) + ", " + str(self.e[2]) + "]"
		
	def __repr__(self):
		return str(self.e[0]) + " " + str(self.e[1]) + " " + str(self.e[2])
		
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
		
	def neg(self):
		return Vec3(-self.e[0],-self.e[1],-self.e[2])

	def __add__(self, other):
		return Vec3(self.e[0]+other.e[0],self.e[1]+other.e[1],self.e[2]+other.e[2])

	def __sub__(self, other):
		return Vec3(self.e[0]-other.e[0],self.e[1]-other.e[1],self.e[2]-other.e[2])

	def __mul__(self, other):
		if type(other) == Vec3:
			return Vec3(self.e[0]*other.e[0],self.e[1]*other.e[1],self.e[2]*other.e[2])
		else:
			return Vec3(self.e[0]*other,self.e[1]*other,self.e[2]*other)

	def __truediv__(self, other):
		if type(other) == Vec3:
			return Vec3(self.e[0]/other.e[0],self.e[1]/other.e[1],self.e[2]/other.e[2])
		else:
			return Vec3(self.e[0]/other,self.e[1]/other,self.e[2]/other)

	def length(self):
		return math.sqrt(self.lengthSquared())

	def lengthSquared(self):
		return self.e[0]*self.e[0]+self.e[1]*self.e[1]+self.e[2]*self.e[2]

	def makeUnitVector(self):
		l = self.length()
		self.e[0] /= l
		self.e[1] /= l
		self.e[2] /= l

	@staticmethod
	def unitVector(v):
		return v / v.length()

	@staticmethod
	def dot(v1,v2):
		return v1.e[0] * v2.e[0] + v1.e[1] * v2.e[1] + v1.e[2] * v2.e[2]
		
	@staticmethod
	def cross(v1,v2):
		return Vec3(v1.e[1] * v2.e[2] - v1.e[2] * v2.e[1],
					v1.e[2] * v2.e[0] - v1.e[0] * v2.e[2],
					v1.e[0] * v2.e[1] - v1.e[1] * v2.e[0])

	@staticmethod
	def reflect(v, n):
		return v - n*2*Vec3.dot(v,n)

	@staticmethod
	def refract(uv, n, etaiOverEtat):
		cosTheta            = min(-Vec3.dot(uv, n), 1.0)
		rOutParallel        = (uv + n*cosTheta) * etaiOverEtat
		rOutPerpendicular   = n * -math.sqrt(1.0 - rOutParallel.lengthSquared())
		return rOutParallel + rOutPerpendicular
		
	@classmethod
	def random(cls, min=0, max=1):
		return cls(random.uniform(min,max), random.uniform(min,max), random.uniform(min,max))
			
	@classmethod
	def randomInUnitDisk(cls):
		while True:
			p = cls(random.uniform(-1,1), random.uniform(-1,1), 0)
			if p.lengthSquared() >= 1: continue
			return p

	@classmethod
	def randomUnitVector(cls):
		a = random.uniform(0, 2*math.pi)
		z = random.uniform(-1, 1)
		r = math.sqrt(1 - z*z)
		return cls(r*math.cos(a), r*math.sin(a), z)

	@classmethod
	def randomInUnitSphere(cls):
		while True:
			p = cls.random(-1,1)
			if p.lengthSquared() >= 1: continue
			return p

	@classmethod
	def randomInHemisphere(cls,normal):
		inUnitSphere = randomInUnitSphere()
		if cls.dot(inUnitSphere, normal) > 0.0: # In the same hemisphere as the normal
			return inUnitSphere
		else:
			return inUnitSphere*-1
