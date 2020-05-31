import png

def mandelbrot(c):
	z = complex(0,0);
	depth = 0;
	while (depth < 255 and abs(z) < 2):
		z = z*z+c
		depth+=1
	return depth
	
def toColor(buf, i, d):
	buf[i+0] = v
	buf[i+1] = (v*3)%256
	buf[i+2] = (v*25)%256
	buf[i+3] = 255
	
width = 128
height = 128
buf = bytearray(width*height*4)

dx = 2.8 / width;
dy = 2.6 / height;
xs = -2.1;
ys = -1.3;
i = 0

for y in range(0,height):
	for x in range(0,width):
		c = complex(xs + dx*x,ys + dy*y)
		v = mandelbrot(c)
		toColor(buf, i, v)
		i += 4

png.save("fractal.png", buf, width, height)
