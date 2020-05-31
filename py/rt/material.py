import math
import random
from abc import ABC, abstractmethod

from vec3 import vec3
from ray import ray


class material(ABC):
	@abstractmethod
	def scatter(self, rIn, hitRec):
		pass
		
class lambertian(material):
	
	def __init__(self, color):
		self.color = color	
	
	def scatter(self, rIn, hitRec):
		scatteredDirection = hitRec.n + vec3.randomPointInSphere()
		scatterRay = ray(hitRec.p, scatteredDirection)
		return (self.color, scatterRay)
		
		
class metal(material):
	def __init__(self, color, fuzz):
		self.color = color
		self.fuzz = fuzz
		
	def scatter(self, rIn, rec):
		reflected = vec3.reflect(vec3.normalize(rIn.direction()), rec.n)
		scattered = ray(rec.p, reflected + vec3.randomPointInSphere()*self.fuzz)
		if vec3.dot(scattered.direction(), rec.n) > 0:
			return (self.color, scattered)
		else:
			return None
		
		
class dielectric(material):
	def __init__(self, refIndex):
		self.refIndex = refIndex
		
	@staticmethod
	def schlick(cosine, refIndex):	
		r0 = (1-refIndex) / (1+refIndex)
		r0 = r0*r0
		return r0 + (1-r0)*((1 - cosine)**5)
		
	def scatter(self, rIn, rec):
		attenuation = vec3(1,1,1)			
		etaiOverEtat = (1.0 / self.refIndex) if rec.frontFace else self.refIndex

		unitDirection = vec3.normalize(rIn.direction())
		cosTheta = min(vec3.dot(unitDirection*-1, rec.n), 1.0)
		sinTheta = math.sqrt(1.0 - cosTheta*cosTheta)
		
		# TIR
		if etaiOverEtat * sinTheta > 1.0:
			reflected = vec3.reflect(unitDirection, rec.n)
			scattered = ray(rec.p, reflected)
			return (attenuation, scattered)
		
		# prop. reflection / refraction
		reflectProb = dielectric.schlick(cosTheta, etaiOverEtat)
		
		# reflection
		if random.random() < reflectProb:
			reflected = vec3.reflect(unitDirection, rec.n)
			scattered = ray(rec.p, reflected)
			return (attenuation, scattered)
			
		# refraction	
		refracted = vec3.refract(unitDirection, rec.n, etaiOverEtat)
		scattered = ray(rec.p, refracted)
		return (attenuation, scattered)		
		