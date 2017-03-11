#!/usr/bin/env python
import sys

lines = [line.rstrip('\n') for line in open("logLikelihoods.txt")]

sum = 0
for i in lines:
	sum = sum + float(i)
	
print "PCFG average log likelihood:",sum/int(sys.argv[1])