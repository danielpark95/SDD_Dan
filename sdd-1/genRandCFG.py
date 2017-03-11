#!/usr/bin/env python
import random
import sys

nonTerms = 5
count = 50
rules = []
i=0

while i < count:
	theVal = 0
	theRand = random.random()*100
	incrementor = float(100)
	incrementor = incrementor/nonTerms
	round = 0
	cieling = 0
	while round < 100:
		cieling += incrementor
		if theRand < cieling:
			theVal += 100*round
			break
		round += 1
	theRand = random.random()*100
	round = 0
	cieling = 0
	while round < 100:
		cieling += incrementor
		if theRand < cieling:
			theVal += 10*round
			break
		round += 1
	theRand = random.random()*100
	round = 0
	cieling = 0
	while round < 100:
		cieling += incrementor
		if theRand < cieling:
			theVal += round
			break
		round += 1
	if rules.count(theVal) == 0:
		rules.append(theVal)
		i += 1
		
for i in rules:
	sys.stdout.write("%d," % i)
	