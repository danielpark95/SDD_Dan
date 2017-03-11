#!/bin/bash

gcc -O2 -std=c99 parse.c -Iinclude -Llib -lsdd -lm -o parse
for ((run=19;run<21;run++))
do
	./parse $run
done