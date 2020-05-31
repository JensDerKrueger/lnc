class HitableList:
	def __init__(self):
		self.objects = []
		
	def add(self, object):
		self.objects.append(object)

	def hit(self, r, tMin, tMax):		
		hit = None
		for object in self.objects:
			tmpHit = object.hit(r, tMin, tMax)
			if tmpHit != None:
				tMax = tmpHit.t
				hit = tmpHit
		return hit

