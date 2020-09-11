import sys

digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

def reverse(inStr):
	out = ""	
	for index in range(len(inStr)):
		out = inStr[index] + out
	return out

def find(c, inStr):
	for index in range(len(inStr)):
		if c == inStr[index]: return index
	return 255

def strToInt(inStr, inBase):
	number = 0
	multi = 1
	revInStr = reverse(inStr)
	for index in range(len(inStr)):
		digit = find(revInStr[index], digits)
		number += digit * multi
		multi *= inBase
	return number

def intToStr(number, outBase):
	revOutStr = ""
	if number == 0: return "0"	
	while number > 0:
		digit = number % outBase
		revOutStr += digits[digit]
		number = number // outBase
	return reverse(revOutStr)

def convert(inStr, inBase, outBase):
	return intToStr(strToInt(inStr,inBase),outBase)

print(sys.argv[1], "->", convert(sys.argv[1], strToInt(sys.argv[2],10), strToInt(sys.argv[3], 10)))
