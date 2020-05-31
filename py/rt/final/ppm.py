def save(filename, data, width, height):
	f = open(filename, "w")
	f.write("P3\n")
	f.write(str(width)+" "+str(height)+" 255\n")
	
	for y in range(0,height):
		for x in range(0,width):	
			index = x + (height-(y+1)) * width
			f.write(str(int(data[index][0])) + " " + str(int(data[index][1])) + " " + str(int(data[index][2])) + "\n")
		
	f.close()
	
