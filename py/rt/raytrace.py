import ppm, png
from vec3 import vec3
from ray import ray
from sphere import sphere
from hitable import *
from hitableList import hitableList
from material import *
from camera import camera

def clamp(val, minValue=0, maxValue=1):
	return max(min(val, maxValue), minValue)
	
def lerp(t, a, b):
	return a*(1-t)+b*t

def rayColor(r, scene, depth):
	
	if depth == 0:
		return vec3(0,0,0)
		
	hit = scene.hit(r, 0.0001, float("inf"))
	if hit != None:
		result = hit.mat.scatter(r, hit)
		if result != None:
			return rayColor(result[1], scene, depth-1)* result[0]

	t = (vec3.normalize(r.direction()).y() + 1)*0.5
	return vec3(1.0, 1.0, 1.0)*(1.0-t) + vec3(0.5, 0.7, 1.0)*t

def generateScene():
	world = hitableList()
		
	# Bottom "plane"
	world.add(sphere(vec3(0,-1000,0), 1000, lambertian(vec3(0.5, 0.5, 0.5))))

	# three large spheres
	world.add(sphere(vec3(0, 1, 0), 1.0, dielectric(1.5)))
	world.add(sphere(vec3(-4, 1, 0), 1.0, lambertian(vec3(0.4, 0.2, 0.1))))
	world.add(sphere(vec3(4, 1, 0), 1.0, metal(vec3(0.7, 0.6, 0.5), 0.0)))
	
	# numerous small spheres
	i = 1
	for a in range(-11, 11):
		for b in range(-11, 11):
			chooseMat = random.random()
			center = vec3(a + 0.9*random.random(), 0.2, b + 0.9*random.uniform(0,1))
			if (center - vec3(4, 0.2, 0)).length() > 0.9:
				if chooseMat < 0.8:
					# diffuse
					albedo = vec3.random() * vec3.random()
					world.add(sphere(center, 0.2, lambertian(albedo)))
				elif chooseMat < 0.95:
					# metal
					albedo = random.uniform(.5, 1)
					fuzz = random.uniform(0, .5)
					world.add(sphere(center, 0.2, metal(albedo, fuzz)))
				else:
					# glass
					world.add(sphere(center, 0.2, dielectric(1.5)))	
					
					
	return world


width = 200
aspectRatio = 16 / 9
height= int(width / aspectRatio)
image = width*height*[None]

maxDepth = 10
sampleCount = 1

lookFrom = vec3(13,2,3)
lookAt = vec3(0,0,0)
vUp = vec3(0,1,0)
distToFocus = 10
aperture = 0 # 0.1
fovy = 20

myCamera = camera(lookFrom, lookAt, vUp, fovy, aspectRatio, aperture, distToFocus)
scene = generateScene()

i = 0
for y in range(height):
	print("Berechne Zeile", y)
	for x in range(width):
		c = vec3(0,0,0)
		for s in range(sampleCount):
			u = (x+random.random())/(width-1)
			v = (y+random.random())/(height-1)	
			primary = myCamera.getRay(u,v)
			c += rayColor(primary, scene, maxDepth)/sampleCount
		image[i] = c
		i += 1
		
png.save("test.ppm", image, width, height)