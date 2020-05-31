save#!/usr/bin/env python3

import png  # writing pngs
import sys  # for the cmd line params

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
		
def progress(value, length):
	s = ""
	progress = int(value*length)
	for i in range(0,length):
		if i < progress:
			s+=chr(9609)
		else:
			s+=chr(9617)
	s += " " + str(int(value*100)) + "%\r"
	print(s, end='', flush=True)

width = 128
height = 128
filename = "fractal.png"

if len(sys.argv) == 4:
	try:
		width = int(sys.argv[1])
		height = int(sys.argv[2])
		filename = sys.argv[3]
	except:
		print("Invalid size parameters, using defaults")
		width = 128
		height = 128
else:
	print("Invalid parameter count, using defaults")

buf = bytearray(width*height*4)

print("Computing fractal of size " + str(width) + "x" + str(height))


dx = 2.8 / width;
dy = 2.6 / height;
xs = -2.1;
ys = -1.3;
i = 0
for y in range(0,height):
	progress((y+1)/height, 50)
	for x in range(0,width):
		c = complex(xs + dx*x,ys + dy*y)
		v = mandelbrot(c)
		toColor(buf, i, v)
		i += 4

print("\nDone! Now Saving " + filename)

try:
	png.save(filename, buf, width, height)
except:
	print("Error saving " + filename)
