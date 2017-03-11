#!/bin/bash

#PCFG
printf "************************************\n"
printf "* * * * *       PCFG       * * * * *\n"
printf "************************************\n"

# Step 1: This reads all the sample sentences in a given grammar
# and assigns a probability distribution over all production
# rules. Saves the output to weights_updated.lt
cd inside-outside
printf "Step 1: Inside-outside.\n\n"
printf "Input: rules.lt, training_data.yld\n"
printf "Output: weights.lt\n\n"
printf "Weights:\n"
./io -d 1000 -g ../grammar/rules.lt ../grammar/training_data.yld | tee ../grammar/weights.lt

# Step 2: This takes weights_updated.lt that's output by inside-outside
# and parses the sentences. Saves the output to parses.txt
cd ../cky
printf "Step 2: CKY.\n\n"
printf "Input: testing_data.yld, weights.lt\n"
printf "Output: parses.txt\n\n"
printf "Parses:\n"
./lncky ../grammar/testing_data.yld 0 ../grammar/weights.lt | tee ../grammar/parses.txt
printf "\n"

# Step 3: This computes the log likelihood 
printf "Step 3: Computing Log Likelihood.\n\n"
python2 computeTot.py 1000

#PSDD
printf "************************************\n"
printf "* * * * *       PSDD       * * * * *\n"
printf "************************************\n"

# Step 1: Encode CFG into SDD. Had to rewrite transBoth function and change
# implementation details to allow for English grammar. Followed Noah's format
# for entering rules and terminals.
printf "Step 1: Compile and run my_parse.c.\n\n"
cd ..
gcc -O2 -std=c99 my_parse.c -Iinclude -Llib -lsdd -lm -o my_parse
./my_parse
printf "\n"

printf "Step 2: Generate dataset and train.\n\n"
cd psdd2
python2 testing-dan2.py
python2 psddSetToPcfg.py
printf "\n"

printf "Step 3: Test and output loglikelihood.\n\n"
cd ../psdd
python2 ./testing-dan.py 1 ../psdd2/psddGeneratedSets/
python2 outputLL.py




