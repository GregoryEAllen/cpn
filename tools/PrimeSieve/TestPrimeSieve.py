#!/usr/bin/python

import string,math,os

#-----------------------------------------------------------------------------
def primes(n): 
#-----------------------------------------------------------------------------
	if n==2: return [2]
	elif n<2: return []
	s=range(3,n+1,2)
	mroot = n ** 0.5
	half=(n+1)/2-1
	i=0
	m=3
	while m <= mroot:
		if s[i]:
			j=(m*m-3)/2
			s[j]=0
			while j<half:
				s[j]=0
				j+=m
		i=i+1
		m=2*i+3
	return [2]+[x for x in s if x]

#-----------------------------------------------------------------------------
def isprime(aNumber):
#-----------------------------------------------------------------------------
	'''return True if the number is prime, false otherwise'''
	if aNumber < 2: return False
	if aNumber == 2: return True
	if (( aNumber / 2 ) * 2 == aNumber) : 
		return False
	else:
		klist = primes(int(math.sqrt(aNumber+1)))
		for k in klist[1:]:
			if (( aNumber / k ) * k == aNumber ): return False
		return True

#print primes(13)
#print primes(3000)

os.system("./PrimeSieve | grep -v \"^#\" > myPrimes.txt")

#-----------------------------------------------------------------------------
# load the file "primes.txt" into the list "thePrimes"
f = open("myPrimes.txt")
try:
	primeStr = f.readlines()
finally:
	f.close()
myPrimes = []
for line in primeStr:
	myPrimes.append( string.atoi(line) )
#print myPrimes
#-----------------------------------------------------------------------------

# check that myPrimes matches thePrimes
thePrimes = primes(myPrimes[-1])
if len(myPrimes)!=len(thePrimes):
	print "len(myPrimes)!=len(thePrimes)", len(myPrimes), len(thePrimes)

errCount = 0
for i in range(len(myPrimes)):
	if myPrimes[i] != thePrimes[i]:
#		print "Error: non-matching numbers", myPrimes[i], thePrimes[i]
		errCount += 1


if errCount>0:
	print errCount, "errors"
	os.system("diff myPrimes.txt thePrimes.txt")
else:
	print "passed"
