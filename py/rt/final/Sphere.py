from Hitable import Hitable, HitRecord
from Vec3 import Vec3
from Ray import Ray
from math import sqrt

class Sphere(Hitable):
	def __init__(self, center, radius, mat):
		self.center = center
		self.radius = radius
		self.mat = mat

	def hit(self, r, tMin, tMax):
		a = r.direction().lengthSquared()
		oc = r.origin() - self.center
		halfB = Vec3.dot(oc, r.direction())
		c = oc.lengthSquared() - self.radius*self.radius
		discriminant = halfB*halfB - a*c

		if discriminant > 0:
			root = sqrt(discriminant)

			t = (-halfB - root) / a
			if t < tMax and t > tMin:
				p = r.pointAtParameter(t)
				outwardNormal = (p - self.center) / self.radius
				return HitRecord(t,p,self.mat,r,outwardNormal)
			
			t = (-halfB + root) / a
			if t < tMax and t > tMin:
				p = r.pointAtParameter(t)
				outwardNormal = (p - self.center) / self.radius
				return HitRecord(t,p,self.mat,r,outwardNormal)
		
		return None		
		
