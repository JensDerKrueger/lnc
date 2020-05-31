from vec3 import vec3

class ray:
	def __init__(self, o, d):
		self.o = o
		self.d = d

	def __str__(self):
		return "{" + str(self.o) + " + " + str(self.d) + " * t}"

	def origin(self):
		return self.o
		
	def direction(self):
		return self.d
		
	def pointAtParameter(self, t):
		return self.o + self.d*t
		
		
if __name__ == "__main__":
	x = ray(vec3(0,0,0), vec3(0,0,-1))
	print(x)	