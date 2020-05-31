from abc import ABC, abstractmethod
from Vec3 import Vec3

class HitRecord:
	def __init__(self, t, p, r, outwardNormal):
		self.t = t
		self.p = p
		
		self.frontFace = Vec3.dot(r.direction(), outwardNormal) < 0
		self.normal = outwardNormal if self.frontFace else outwardNormal*-1
	

	def __str__(self):
		return "t="+repr(self.t) + " p:" + repr(self.p) + " normal:" + repr(self.normal)
		
class Hitable(ABC):	
	@abstractmethod
	def hit(r, tMin, tMax):  # returns hitRecord or None of no hit is found
		pass
		