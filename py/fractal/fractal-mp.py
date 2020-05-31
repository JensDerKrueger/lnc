#!/usr/bin/env python3

import png  # writing pngs
import sys  # for the cmd line params
import time  # performance measurement
from multiprocessing import Process, Array
	
width = 512
height = 512
dx = 2.8 / width
dy = 2.6 / height
xs = -2.1
ys = -1.3
filename = "fractal.png"
processCount = 2
	
def compute(c):
	z = complex(0,0)
	depth = 0
	while depth < 255 and abs(z) < 2:
		z = z*z+c
		depth+=1
	return depth

def writePixel(buf, pos, v):
	buf[pos*4+0] = v
	buf[pos*4+1] = (v*3)%256
	buf[pos*4+2] = (v*25)%256
	buf[pos*4+3] = 255

def computePortion(starty, width, height, step, buf):	
	for y in range(starty, height, step):
		for x in range(0,width):
			c = complex(xs + dx*x,ys + dy*y)
			v = compute(c)
			buf[x+y*width] = v

def progress(value, length=50):
	s = ""
	progress = int(value*length)
	for i in range(0,length):
		if i < progress:
			s+=chr(9609)
		else:
			s+=chr(9617)
	s += " " + str(int(value*100)) + "%\r"
	print(s, end='', flush=True)


	
buf = bytearray(width*height*4)

start = time.perf_counter()

if processCount == 1:
	print("Computing fractal of size " + str(width) + "x" + str(height) + " using only a single process")
	pixel = 0
	for y in range(0,height):
		progress((y+1)/height)
		for x in range(0,width):
			c = complex(xs + dx*x,ys + dy*y)
			v = compute(c)
			writePixel(buf, pixel, v)
			pixel += 1
else:
	print("Computing fractal of size " + str(width) + "x" + str(height) + " using " + str(processCount) + " processes")
	try:
		arr = Array('B', height*width) 
		processes = []
		for i in range(processCount):
			p = Process(target=computePortion, args=(i, width, height, processCount, arr))
			p.start()
			processes.append(p)
		
		for p in processes:
			p.join()			
		
		print("Buffer to Color Conversion")
		for i in range(height*width):
			writePixel(buf, i, arr[i])
		

	except Exception as e:
		print ("Error: Process Error: " + e)
		quit()

	

end = time.perf_counter()

print("\nDone after " + str(int(end-start)) + " seconds. Now Saving " + filename)

try:
	png.save(filename, buf, width, height)
except:
	print("Error saving " + filename)
