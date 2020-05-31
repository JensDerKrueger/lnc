def compress(buf, width, height):
	import zlib, struct
	width_byte_4 = width * 4
	raw_data = b''.join(b'\x00' + buf[span:span + width_byte_4]
						for span in range((height - 1) * width_byte_4, -1, - width_byte_4))
	def png_pack(png_tag, data):
		chunk_head = png_tag + data
		return (struct.pack("!I", len(data)) +
				chunk_head +
				struct.pack("!I", 0xFFFFFFFF & zlib.crc32(chunk_head)))
	return b''.join([
		b'\x89PNG\r\n\x1a\n',
		png_pack(b'IHDR', struct.pack("!2I5B", width, height, 8, 6, 0, 0, 0)),
		png_pack(b'IDAT', zlib.compress(raw_data, 9)),
		png_pack(b'IEND', b'')])
		
def saveBuffer(filename, buf, width, height):
	data = compress(buf, width, height)
	with open(filename, 'wb') as fd:
		fd.write(data)
		
def save(filename, data, width, height): 
	array = bytearray(width*height*4)
	i = 0	
	for v in data:
		array[i] = v[0]; i += 1
		array[i] = v[1]; i += 1
		array[i] = v[2]; i += 1
		array[i] = 255;  i += 1
	saveBuffer(filename,array,width,height)