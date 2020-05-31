import time

class Stopwatch:
	def __init__(self):
		self.start = time.perf_counter()
	
	def elapsedTime(self):
		return time.perf_counter() - self.start