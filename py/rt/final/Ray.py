class Ray:
	def __init__(self, a, b):
		self.a = a
		self.b = b

	def origin(self):
		return self.a

	def direction(self):
		return self.b
		
	def pointAtParameter(self, t):
		return self.a + self.b*t
		
	def __str__(self):
		return "{" + str(self.a) + " + t" + str(self.b) + "}"