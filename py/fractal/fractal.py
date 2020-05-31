import png
import Stopwatch

def mandelbrot(c):
	z = complex(0,0)
	i = 0
	while i <= 255 and abs(z) < 2:
		z = z*z+c
		i += 1
	return i
	
	
def indexToColor(picture, index, i):
	picture[index+0] = (i*1)%256
	picture[index+1] = (i*3)%256
	picture[index+2] = (i*25)%256
	picture[index+3] = 255

width = 4096
height = 4096

picture = bytearray(width*height*4)

zoom = 1
xs = -2.1
ys = -1.3


s = Stopwatch.Stopwatch()

dx = (2.6/zoom) / width
dy = (2.6/zoom) / height
i = 0
for y in range(height):
	for x in range(width):
		c = complex(xs+x*dx,
					ys+y*dy)
		colorIndex = mandelbrot(c)
		indexToColor(picture, i*4, colorIndex)
		i += 1

print("Elapsed ms:", s.elapsedTime()*1000)		
		
		
png.save("fractal.png", picture, width, height)