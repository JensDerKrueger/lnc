from vec3 import vec3
import math

def save(filename, data, width, height):
	f = open(filename, "w")
	
	f.write("P3\n")
	f.write(str(width) + " " + str(height) + " 255\n")
	
	for y in range(0,height):
		for x in range(0,width):
			index = x + (height-(y+1)) * width
			p = data[index]
			
			r = math.sqrt(p.r())
			g = math.sqrt(p.g())
			b = math.sqrt(p.b())
			
			f.write(str(int(r*255)) + " " + str(int(g*255)) + " " + str(int(b*255)) + "\n")
	
	f.close()
