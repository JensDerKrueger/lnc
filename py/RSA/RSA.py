from random import randint

def isPrime(n, k=8):
    if n < 2: return False
    for p in [2,3,5,7,11,13,17,19,23,29,31,37]:
        if n % p == 0: return n == p
    s, d = 0, n-1
    while d % 2 == 0:
        s, d = s+1, d//2
    for i in range(k):
        x = pow(randint(2, n-1), d, n)
        if x == 1 or x == n-1: continue
        for r in range(1, s):
            x = (x * x) % n
            if x == 1: return False
            if x == n-1: break
        else: return False
    return True

def genPrime(a=10**400,b=10**600):
    p = randint(a,b)
    while not isPrime(p):
        p = randint(a,b)
    return p

def extendedEuclid(a,b):
    if b == 0: return (a,1,0)
    (dp, sp, tp) = extendedEuclid(b, a % b)
    (d,s,t) = (dp, tp, sp - (a // b) * tp)
    return (d,s,t)

def genRSAKey(a=10**100,b=10**150):
    p = genPrime(a,b)
    q = genPrime(a,b)
    n = p*q
    phi = (p-1)*(q-1)
    
    e = randint(2,phi-1)
    (gcd, d, t) = extendedEuclid(e,phi)
    while gcd != 1: 
        e = randint(2,phi-1)
        (gcd, d, t) = extendedEuclid(e,phi)
    
    return ((e,n),(d,n))

def strToInt(s):
    i = 0
    for d in range(len(s)):
        o = ord(s[d])
        i = i * 256 + o
    return i

def intToStr(i):
    s = ""
    while i > 0:
        o = chr(i%256)
        s = o + s
        i = i // 256
    return s

(publicKey, privateKey) = genRSAKey()
print("Public Key:", publicKey)
print("Private Key:", privateKey)
print()

plainText = "Hello World"
encoded = pow(strToInt(plainText), publicKey[0], publicKey[1])
decoded = intToStr(pow(encoded, privateKey[0], privateKey[1]))

print("Plain text:", plainText)
print("Encrypted text:", encoded)
print("Decrypted text:", decoded)
