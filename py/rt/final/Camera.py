import math
import random
from Vec3 import Vec3
from Ray import Ray


class Camera:
	def __init__(self, origin, lookat, vup, vfov, aspectRatio, aperture, focusDist):
		self.origin = origin
		self.lensRadius = aperture / 2
		self.theta = math.radians(vfov)
		self.halfHeight = math.tan(self.theta/2)
		self.halfWidth = aspectRatio * self.halfHeight
		self.w = Vec3.unitVector(lookat-origin)
		self.u = Vec3.unitVector(Vec3.cross(self.w,vup))
		self.v = Vec3.cross(self.u,self.w)
		self.lowerLeftCorner = origin-self.u*self.halfWidth*focusDist-self.v*self.halfHeight*focusDist+self.w*focusDist
		self.horizontal = self.u*self.halfWidth*focusDist*2
		self.vertical = self.v*self.halfHeight*focusDist*2
	
	def getRay(self, s, t):
		rd = Vec3.randomInUnitDisk()*self.lensRadius
		offset = self.u * rd.x() + self.v * rd.y()
		return Ray(self.origin + offset, self.lowerLeftCorner + self.horizontal*s + self.vertical*t - self.origin - offset)