from math import gcd;

class Fraction():
	def __init__(self,n,d):
		self.__n = n
		self.__d = d
		self.__cancelDown()
		
	def __cancelDown(self):
		if self.__n == 0:
			self.__d = 1
			return
		
		s = -1 if self.__n*self.__d < 0 else 1
		n = abs(self.__n)
		d = abs(self.__d)
		
		g = gcd(n,d)
		self.__n = s * n//g
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
		
		if n*d < 0:
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

	def isValid(self):
		return self.__d != 0

print(Fraction(3,4) + (1,2))

a = Fraction(1,2)
b = Fraction(1,0)

print(a.isValid())
print(b.isValid())
