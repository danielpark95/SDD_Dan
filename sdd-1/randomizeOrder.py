#!/usr/bin/env python
from random import randint
import sys

L1 = [12,14,21,50,30,41,40,23,13,43,1,53,24,34,54,51,31,32,52,3,2,4,10,42,20,16,26,36,46,56,5,15,25,35,45]
LLen = len(L1)
L2 = []
for i in range(LLen):
	L2.append(L1.pop(randint(0,len(L1)-1)))
	
for i in L2:
	sys.stdout.write("%d," % i)
	
