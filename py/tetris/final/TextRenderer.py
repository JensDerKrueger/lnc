from Renderer import Renderer

class TextRenderer(Renderer):
	def setColor(self,rgb):
		index = 16 + int(rgb[2]*5) + 6*int(rgb[1]*5) + 36*int(rgb[0]*5)
		print("\033[48;5;"+str(index)+"m", end="")

	def render(self, data):
		print("\033[2J\033[;H")
		i = 0
		for y in range(self.height):
			for x in range(self.width):
				rgb = data[i]
				self.setColor(data[i])
				i += 1
				print("  ", end="")
			self.setColor([0,0,0])
			print("")
