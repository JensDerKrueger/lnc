from hitable import *

class hitableList:
	def __init__(self):
		self.objects = []
		
	def add(self, o):
		self.objects.append(o)
		
	def hit(self, r, tMin, tMax):
		hit = None
		
		for object in self.objects:
			currentHit = object.hit(r, tMin, tMax)
			if currentHit != None:
				tMax = currentHit.t
				hit = currentHit
				
		return hit
		