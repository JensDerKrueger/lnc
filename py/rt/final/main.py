import random, sys

import png
from Stopwatch import Stopwatch

from Sphere import Sphere
from Material import *
from HitableList import HitableList
from Vec3 import Vec3
from Camera import Camera

lastLength = 0
def showProgress(stopwatch, j, imageHeight):
	global lastLength
	s = "\r" + " "*lastLength
	print(s,end="\r")	
	s = "Completed: {0:2.2f}% (elapsed time: {1:2.2f} sec.)".format(100*j/imageHeight, stopwatch.elapsedTime())
	lastLength = len(s)
	print(s, end="")
	sys.stdout.flush()

def rayColor(r, world, depth):
	# if we've exceeded the ray bounce limit, no more light is gathered.
	if depth <= 0:
		return Vec3(0,0,0)

	rec = world.hit(r, 0.001, float('inf'))
	if rec != None:		
		result = rec.mat.scatter(r, rec)
		if result != None:
			attenuation = result[0]
			scattered = result[1]
			return rayColor(scattered, world, depth-1)*attenuation
		return Vec3(0,0,0)

	# background
	unitDirection = Vec3.unitVector(r.direction())
	t = (unitDirection.y() + 1.0)*0.5
	return Vec3(1.0, 1.0, 1.0)*(1.0-t) + Vec3(0.5, 0.7, 1.0)*t


def randomSphereScene():
	world = HitableList()

	# Bottom "plane"
	world.add(Sphere(Vec3(0,-1000,0), 1000, Lambertian(Vec3(0.5, 0.5, 0.5))))

	# three large spheres
	world.add(Sphere(Vec3(0, 1, 0), 1.0, Dielectric(1.5)))
	world.add(Sphere(Vec3(-4, 1, 0), 1.0, Lambertian(Vec3(0.4, 0.2, 0.1))))
	world.add(Sphere(Vec3(4, 1, 0), 1.0, Metal(Vec3(0.7, 0.6, 0.5), 0.0)))

	# numerous small spheres
	i = 1
	for a in range(-11, 11):
		for b in range(-11, 11):
			chooseMat = random.random()
			center = Vec3(a + 0.9*random.random(), 0.2, b + 0.9*random.uniform(0,1))
			if (center - Vec3(4, 0.2, 0)).length() > 0.9:
				if chooseMat < 0.8:
					# diffuse
					albedo = Vec3.random() * Vec3.random()
					world.add(Sphere(center, 0.2, Lambertian(albedo)))
				elif chooseMat < 0.95:
					# metal
					albedo = random.uniform(.5, 1)
					fuzz = random.uniform(0, .5)
					world.add(Sphere(center, 0.2, Metal(albedo, fuzz)))
				else:
					# glass
					world.add(Sphere(center, 0.2, Dielectric(1.5)))

	return world

def clamp(n, smallest, largest): 
	return max(smallest, min(n, largest))
	
def vecToColor(pixelColor, samplesPerPixel):
	r = pixelColor.r()
	g = pixelColor.g()
	b = pixelColor.z()

	# Replace NaN components with zero. See explanation in Ray Tracing: The Rest of Your Life.
	if r != r: r = 0.0
	if g != g: g = 0.0
	if b != b: b = 0.0

	# Divide the color by the number of samples and gamma-correct for gamma=2.0.
	scale = 1.0 / samplesPerPixel
	r = math.sqrt(scale * r)
	g = math.sqrt(scale * g)
	b = math.sqrt(scale * b)
	
	# return the translated [0,255] value of each color component.
	return [int(256 * clamp(r, 0.0, 0.999)),
			int(256 * clamp(g, 0.0, 0.999)),
			int(256 * clamp(b, 0.0, 0.999))]


if __name__ == "__main__":
	aspectRatio = 16.0 / 9.0
	imageWidth = 500
	imageHeight = int(imageWidth / aspectRatio)
	samplesPerPixel = 20
	maxDepth = 50

	world = randomSphereScene()

	lookFrom = Vec3(13,2,3)
	lookAt = Vec3(0,0,0)
	vUp = Vec3(0,1,0)
	distToFocus = 10.0
	aperture = 0.1

	cam = Camera(lookFrom, lookAt, vUp, 20, aspectRatio, aperture, distToFocus)

	stopwatch = Stopwatch()
	image = imageHeight*imageWidth*[[0,0,0]]
	index = 0
	for j in range(imageHeight):
		showProgress(stopwatch, j, imageHeight)
		for i in range(imageWidth):
			pixelColor = Vec3(0,0,0)
			for s in range(samplesPerPixel):
				u = (i + random.random()) / (imageWidth-1)
				v = (j + random.random()) / (imageHeight-1)
				r = cam.getRay(u, v)
				pixelColor = pixelColor + rayColor(r, world, maxDepth)
			image[index] = vecToColor(pixelColor, samplesPerPixel)
			index += 1
	
	print("\nTotal time: {0:2.2f} sec.".format(stopwatch.elapsedTime()))	
	print("Saving Image ...")
	png.save("result.png", image, imageWidth, imageHeight)
	print("Done.")