from Renderer import Renderer

class TextRenderer(Renderer):
	def __setColor(self,rgb):
		index = 16 + int(rgb[2]*5) + 6*int(rgb[1]*5) + 36*int(rgb[0]*5)
		print("\033[48;5;"+str(index)+"m", end="")	
	
	def render(self, color):
		print("\033[2J\033[;H")
		
		i = 0
		for y in range(self.height):
			for x in range(self.width):
				rgb = color[i]
				self.__setColor(rgb)
				print("  ", end="")				
				i += 1
			print()
		self.__setColor([0,0,0])
		print()