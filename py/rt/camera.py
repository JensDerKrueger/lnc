import math
from vec3 import vec3
from ray import ray

class camera:
	def __init__(self, lookFrom, lookAt, vUp, fovy, aspectRatio, aperture, distToFocus):
		self.lookFrom = lookFrom
		self.lookAt = lookAt
		self.vUp = vUp
		self.fovy = math.radians(fovy)
		self.aspectRatio = aspectRatio
		self.lenseRadius = aperture/2
		self.distToFocus = distToFocus
		
		self.halfHeight = math.tan(self.fovy/2) * distToFocus
		self.halfWidth  = self.halfHeight * aspectRatio
		
		self.w = vec3.normalize(lookAt-lookFrom)
		self.u = vec3.normalize(vec3.cross(self.w,vUp))
		self.v = vec3.cross(self.u,self.w)

		self.lowerLeftCorner = lookFrom + self.w*distToFocus - self.v*self.halfHeight - self.u*self.halfWidth		
		
	def getRay(self, u, v):		
		temp = vec3.randomPointInDisc() * self.lenseRadius
		apertureOffset = self.u * temp.x() + self.v * temp.y()
		return ray(self.lookFrom + apertureOffset, self.lowerLeftCorner + self.u*u*2*self.halfWidth + self.v*v*2*self.halfHeight - (self.lookFrom + apertureOffset))