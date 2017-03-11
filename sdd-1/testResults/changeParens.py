#!/usr/bin/env python
import sys

filename = sys.argv[1]
f = open(filename,'r+')

lines = f.read()
f.close()
f = open(filename,'w')

for i in lines:
	if i == '(':
		f.write('{')
	elif i == ')':
		f.write('}')
	else:
		f.write(i)
		
		#remember different filename!!!