#!/usr/bin/env python

lines = [line.rstrip('\n') for line in open("logLikelihood.txt")]
print "PSDD average log likelihood:",str(0-float(lines[0]))