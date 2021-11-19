from math import gcd, copysign;

class Fraction():	
	def __init__(self,n,d):
		g = gcd(n,d)
		self.__n = n//g
		self.__d = d//g
		
	def __mul__(self, other):
		return Fraction(self.__n*other.__n, self.__d*other.__d)

	def __add__(self, other):
		return Fraction(self.__n*other.__d + other.__n*self.__d, other.__d*self.__d)
	
	def __sub__(self, other):
		return Fraction(self.__n*other.__d - other.__n*self.__d, other.__d*self.__d)
	
	def __eq__(self, other):
		return self.__n == other.__n and self.__d == other.__d

	def __lt__(self, other):
		return self.__n*other.__d < other.__n*self.__d
	
	def __str__(self):
		if self.__d == 0: return "undefined"
		n = self.__n
		d = self.__d
		
		if copysign(1, n) * copysign(1, d) < 0:
			signChar = "-"
			n = abs(n)
			d = abs(d)
		else:
			signChar = ""
		
		ord = n//d
		if ord > 0:
			n -= ord*d
			ordStr = str(ord) + " "
		else:
			ordStr = ""
			
		if n == 0:
			return signChar + ordStr
		else:
			return signChar + ordStr + str(n) + "/" + str(d)



print (Fraction(3,4) + Fraction(1,4) - Fraction(30,4))