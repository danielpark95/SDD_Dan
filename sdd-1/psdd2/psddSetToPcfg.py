#!/usr/bin/env python


numSets = 100
termCount = 2
length = 5

for run in range(numSets):
	inName = "psddGeneratedSets/psddTraining" + str(run) + ".txt"
	outName = "psddGeneratedSets/pcfgTraining" + str(run) + ".txt"
	termLits = length*termCount

	fIn = open(inName,'r')
	fOut = open(outName,'w')

	lines = fIn.readlines()
	justTLits = []

	for i in lines:
		newList = []
		for char in i:
			if char == '-':
				break
			if char != ',':
				newList.append(char)
		justTLits.append(newList)

	for word in justTLits:
		for j in range(length):
			found = 0
			for k in range(termCount):
				pos = j*termCount+k
				if word[pos] == '1':
					found = k
					break
			if found == 0:
				fOut.write('a')
			elif found == 1:
				fOut.write('b')
			elif found == 2:
				fOut.write('x')
			else:
				fOut.write('y')
			if j != length -1:
				fOut.write(' ')
		fOut.write('\n')
	
	
for run in range(numSets):
	inName = "psddGeneratedSets/psddTesting" + str(run) + ".txt"
	outName = "psddGeneratedSets/pcfgTesting" + str(run) + ".txt"
	termLits = length*termCount

	fIn = open(inName,'r')
	fOut = open(outName,'w')

	lines = fIn.readlines()
	justTLits = []

	for i in lines:
		newList = []
		for char in i:
			if char == '-':
				break
			if char != ',':
				newList.append(char)
		justTLits.append(newList)

	for word in justTLits:
		for j in range(length):
			found = 0
			for k in range(termCount):
				pos = j*termCount+k
				if word[pos] == '1':
					found = k
					break
			if found == 0:
				fOut.write('a')
			elif found == 1:
				fOut.write('b')
			elif found == 2:
				fOut.write('x')
			else:
				fOut.write('y')
			if j != length-1:
				fOut.write(' ')
		fOut.write('\n')


