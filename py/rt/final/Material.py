from abc import ABC, abstractmethod
import math
import random
from Vec3 import Vec3
from Ray import Ray

class Material(ABC):
	
	@abstractmethod
	def scatter(self, rIn, hitRec): # returns (attenuation, scattered ray) or None
		pass


class Dielectric(Material):
	def __init__(self, refIdx):
		self.refIdx = refIdx
		
	@staticmethod
	def __schlick(cosine, refIdx):
		r0 = (1-refIdx) / (1+refIdx)
		r0 = r0*r0
		return r0 + (1-r0)*((1 - cosine)**5)

	def scatter(self, rIn, rec): # returns (attenuation, scattered ray)
		attenuation = Vec3(1.0, 1.0, 1.0)			
		etaiOverEtat = (1.0 / self.refIdx) if rec.frontFace else self.refIdx
		
		unitDirection = Vec3.unitVector(rIn.direction())
		cosTheta = min(Vec3.dot(unitDirection*-1, rec.normal), 1.0)
		sinTheta = math.sqrt(1.0 - cosTheta*cosTheta)
		if etaiOverEtat * sinTheta > 1.0:
			reflected = Vec3.reflect(unitDirection, rec.normal)
			scattered = Ray(rec.p, reflected)
			return (attenuation, scattered)

		reflectProb = Dielectric.__schlick(cosTheta, etaiOverEtat)
		if random.random() < reflectProb:
			reflected = Vec3.reflect(unitDirection, rec.normal)
			scattered = Ray(rec.p, reflected)
			return (attenuation, scattered)
			
			
		refracted = Vec3.refract(unitDirection, rec.normal, etaiOverEtat)
		scattered = Ray(rec.p, refracted)
		return (attenuation, scattered)
		
		
class Lambertian(Material):
	def __init__(self, albedo):
		self.albedo = albedo

	def scatter(self, rIn, rec): # returns (attenuation, scattered)
			scatterDirection = rec.normal + Vec3.randomInUnitSphere()
			scattered = Ray(rec.p, scatterDirection)
			attenuation = self.albedo
			return (attenuation, scattered)

class Metal(Material):
	def __init__(self, albedo, fuzz):
		self.albedo = albedo
		self.fuzz   = fuzz if fuzz < 1 else 1

	def scatter(self, rIn, rec): # returns (attenuation, scattered) or None
		reflected = Vec3.reflect(Vec3.unitVector(rIn.direction()), rec.normal)
		scattered = Ray(rec.p, reflected + Vec3.randomInUnitSphere()*self.fuzz)
		attenuation = self.albedo
		if Vec3.dot(scattered.direction(), rec.normal) > 0:
			return (attenuation, scattered)
		else:
			return None
