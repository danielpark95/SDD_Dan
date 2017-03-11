#!/bin/bash
#manually enter rules to rules.txt

count=100
setSize=8192

cd sdd-1.1.1
gcc -O2 -std=c99 parse.c -Iinclude -Llib -lsdd -lm -o parse
./parse
cd ../generateSet
gcc generateDataset.c -o generate
./generate $count $setSize
cd ../psdd
pypy ./testing-noah.py $count ../generateSet/
cd ..
cd cky
rm logLikelihoods.txt
cd ..

for ((run=0;run<$count;run++))
do
	cd inside-outside
	filename="../generateSet/pcfgTraining"$run".txt"
	./io -d 1000 -g rules.txt $filename > weights.txt
	cd ../cky
	filename="../generateSet/pcfgTesting"$run".txt"
	./lncky $filename 0 ../inside-outside/weights.txt
	cd ..
done
cd cky
python computeTot.py $count
cd ../psdd
python outputLL.py
