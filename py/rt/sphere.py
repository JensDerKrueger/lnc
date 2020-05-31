import math

from vec3 import vec3
from hitable import *

class sphere(hitable):
	def __init__(self, center, radius, material):
		self.center = center
		self.radius = radius
		self.material = material
		
	def hit(self, r, tMin, tMax):
		oc = r.origin()-self.center   # o-c
		
		a = r.direction().squareLength()
		halfB = vec3.dot(r.direction(), oc)
		c = oc.squareLength()-self.radius*self.radius
		
		discriminant = halfB*halfB - a*c
		
		if discriminant >= 0:
			root = math.sqrt(discriminant)
			
			t = (-halfB - root)/a
			
			if t < tMax and t > tMin:			
				intersect = r.pointAtParameter(t)			
				normal = (intersect-self.center)/self.radius
			
				return hitRecord(t, intersect, normal, r, self.material)
		
		return None
		