#include <stdio.h>
#include <stdlib.h>
#include "sddapi.h"

/*
CHANGED THE ORIGINAL CODE TO ALLOW FOR ENGLISH GRAMMAR

LIST OF PRODUCTION RULES
S --> NP VP
NP --> Det N
NP --> N
VP --> V NP
NP --> NP PP
VP --> V PP
VP --> V
PP --> P NP

Det --> a
Det --> the

N --> dog
N --> cat
N --> bone
N --> man
N --> ball
N --> Daniel
N --> soup
N --> table
N --> spoon
N --> food
N --> Mary
N --> John
N --> I
N --> hill
N --> street
N --> lawn
N --> house
N --> school

V --> runs
V --> eats
V --> throws
V --> hikes
V --> bites
V --> drinks
V --> gives
V --> catches
V --> rolls

P --> on
P --> in
P --> to
P --> with
P --> down
P --> at
P --> up

NEW FORMAT WITH TABLE OF ENTRY

	int utNonRules[8] = {812, 145, 15, 261, 113, 263, 26, 371};
	int utTermRules[36] = {40, 41, 52, 53, 54, 55, 56, 57, 58, 59, 
		510, 511, 512, 513, 514, 515, 516, 517, 518, 519, 620, 621, 
		622, 623, 624, 625, 626, 627, 628, 729, 730, 731, 732, 733, 734, 735};

1: NP, 2 : VP, 3: PP, 4: Det, 5: N, 6: V, 7: P, 8: S
0 a, 1 the, 2 dog, 3 cat, 4 bone, 5 man, 6 ball, 7 Daniel, 8 soup, 9 table, 10 spoon, 
11 food, 12 Mary, 13 John, 14 I, 15 hill, 16 street, 17 lawn, 18 house, 19 school, 20 runs, 
21 eats, 22 throws, 23 hikes, 24 bites, 25 drinks, 26 gives, 27 catches, 28 rolls, 29 on, 30 in, 
31 to, 32 with, 33 down, 34 at, 35 up

NOAH'S REPRESENTATION
	Rule representation:
		termRules[][2]
			rule(i) is of the form X->x
			rules[i][0] = X
			rules[i][1] = x
		nonRules[][3]
			rule(i) is of the form X->AB
			rules[i][0] = X
			rules[i][1] = A
			rules[i][2] = B

NOAH'S DEFINITIONS
	String is an array of integers, corresponding to the input string 
	(each integer in string must be < terminal)
	
	# of literals = term*len + nonTerm*len*(len+1)/2
	first term*len literals:
		Terminal i corresponds to S(i/term,i%term)
	next nonTerm*len*(len+1)/2 correspond to non-terminals

	literal for T[i][j][k] = (j*(j+1)/2)*nonTerm+i*nonTerm+k + firstNon

*/

SddNode* sddParsings(SddManager*m, int nonTerm, int term, int nonRules[][3], int termRules[][2], int nonRuleCount, int termRuleCount, int start, int len){
	SddNode* S[term][len];
	int i,j,k,index;
	
	//S holds an entry for each possible terminal in each possible position in the string
	//Each cell holds the positive literal for the indicated terminal in that position conjoined the negative literal for each other terminal in that position
	for(j = 0; j < len; j++){
		for(i = 0; i < term; i++){
			S[i][j] = sdd_manager_literal(j*term+i+1,m);
			for(k = 0; k < term; k++){
				if (k != i){
					S[i][j] = sdd_conjoin(sdd_manager_literal(0-(j*term+k+1),m),S[i][j],m);
				}
			}
		}
	}
	
	int firstNon = term*len+1;
	SddNode* T[len][len][nonTerm];
	
	//set diagonal according to terminal rules
	for (index = 0;index < len; index++){
		//initialize each to false
		for(k = 0; k < nonTerm; k++){
			T[index][index][k] = sdd_manager_false(m);
		}
		//disjoin each possible terminal rule for each terminal in each position in the string	
		for(i = 0; i < termRuleCount; i++){
			k = termRules[i][0];
			int termVal = termRules[i][1];
			T[index][index][k] = sdd_disjoin(T[index][index][k],S[termVal][index],m);
		}
		//conjoin the non-terminal literal for each corresponding cell while conjoining the negative literal for each non-terminal in that position
		for(k = 0; k < nonTerm; k++){
			T[index][index][k] = sdd_conjoin(T[index][index][k],sdd_manager_literal((index*(index+1)/2)*nonTerm+index*nonTerm+k + firstNon,m),m);
			for(j = 0; j < nonTerm; j++){
				if (j != k){
					T[index][index][k] = sdd_conjoin(T[index][index][k],sdd_manager_literal(0-((index*(index+1)/2)*nonTerm+index*nonTerm+j + firstNon),m),m);
				}
			}
		}
	}
	
	SddNode* empties[len][len];
	
	//create an array that holds the conjoined negative non-terminal literals for each non-terminal in a position in the 2D T array
	for (j = 1; j < len; j++){
		for (i = j-1; i >= 0; i--){
			empties[i][j] = sdd_manager_true(m);
			for(k = 0; k < nonTerm; k++){
				empties[i][j] = sdd_conjoin(empties[i][j],sdd_manager_literal(0-((j*(j+1)/2)*nonTerm+i*nonTerm+k + firstNon),m),m);
			}
		}
	}
	
	for (j = 1; j < len; j++){
		for (i = j-1; i >= 0; i--){
			for(k = 0; k < nonTerm; k++){
				T[i][j][k] = sdd_manager_false(m);
			}
			for(index = 0; index < nonRuleCount; index++){
				int root = nonRules[index][0];
				int left = nonRules[index][1];
				int right = nonRules[index][2];
				int up;
				int east = i+1;
				for(up = i; up < j; up++){
					SddNode* delta = sdd_conjoin(T[i][up][left],T[east][j][right],m);
					int midup, mideast, counter;
					for(midup = up+1;midup <= j;midup++){
						for(mideast = i; mideast < east;mideast++){
							if (midup != j || mideast != i){
								delta = sdd_conjoin(delta,empties[mideast][midup],m);
							}
						}
					}
					T[i][j][root] = sdd_disjoin(delta,T[i][j][root],m);
					east+=1;	
				}
			}
			for(k = 0; k < nonTerm; k++){
				T[i][j][k] = sdd_conjoin(T[i][j][k],sdd_manager_literal((j*(j+1)/2)*nonTerm+i*nonTerm+k + firstNon,m),m);
				for(index = 0; index<nonTerm; index++){
					if (index != k){
						T[i][j][k] = sdd_conjoin(T[i][j][k],sdd_manager_literal(0-((j*(j+1)/2)*nonTerm+i*nonTerm+index + firstNon),m),m);
					}
				}
			}		
		}
	}
	
	return T[0][len-1][start];
}

//utn is a set of non terminal rules of the form "121" which translates to 1 -> 2 1, 
//utt is a set of terminal rules of the form "21" which translates to 2 -> 1
//if the non-terminal on the left is 0, change it to 8 -> write 022 as 822.
//Original code had an error for creating excess nodes because of production rules.
void transBoth(int utn[],int nonRules[][3],int nonRuleCount,
	int utt[],int termRules[][2],int termRuleCount){
	int i;
	int term_divisor;

	for (i=0;i < nonRuleCount;i++){
		if (utn[i]/100 == 8) {
			nonRules[i][0] = 0;
			nonRules[i][1] = (utn[i]%100)/10;
			nonRules[i][2] = utn[i]%10;
			//printf("nonrules[%d][0] = %d\n",i,nonRules[i][0]);
			//printf("nonrules[%d][1] = %d\n",i,nonRules[i][1]);
			//printf("nonrules[%d][2] = %d\n",i,nonRules[i][2]);
		}
		else if (utn[i] < 100) {
			nonRules[i][0] = utn[i]/10;
			nonRules[i][1] = utn[i]%10;
			//printf("nonrules[%d][0] = %d\n",i,nonRules[i][0]);
			//printf("nonrules[%d][1] = %d\n",i,nonRules[i][1]);
		}
		else if (utn[i] < 1000){
			nonRules[i][0] = utn[i]/100;
			nonRules[i][1] = (utn[i]%100)/10;
			nonRules[i][2] = utn[i]%10;
			//printf("nonrules[%d][0] = %d\n",i,nonRules[i][0]);
			//printf("nonrules[%d][1] = %d\n",i,nonRules[i][1]);
			//printf("nonrules[%d][2] = %d\n",i,nonRules[i][2]);
		}
	}
	for (i=0;i<termRuleCount;i++){
		//printf("terminal rule = %d\n",utt[i]);
		if (utt[i] < 100)
			term_divisor = 10;
		else
			term_divisor = 100;
		termRules[i][0] = utt[i]/term_divisor;
		termRules[i][1] = utt[i]%term_divisor;

		//printf("termrules[%d][0] = %d\n",i,termRules[i][0]);
		//printf("termrules[%d][1] = %d\n",i,termRules[i][1]);
	}
}

int main(int argc, char** argv) {
	// Hard coded the production rules to see if we can compile English.

	// nonterminals are S, NP, VP, PP, Det, N, V, P
	int nonTerm = 8;
	// terminals are 0 a, 1 the, 2 dog, 3 cat, 4 bone, 5 man, 6 ball, 7 Daniel, 8 soup, 
	// 9 table, 10 spoon, 11 food, 12 Mary, 13 John, 14 I, 15 hill, 16 street, 17 lawn, 
	// 18 house, 19 school, 20 runs, 21 eats, 22 throws, 23 hikes, 24 bites, 25 drinks, 
	// 26 gives, 27 catches, 28 rolls, 29 on, 30 in, 31 to, 32 with, 33 down, 34 at, 35 up
	int term = 36;
	int nonRuleCount = 8;
	int termRuleCount = 36;
	// min sentence length
	int start = 2;
	// max sentence length
	int len = 7;

	//arrays for storing CFG
	int nonRules[nonRuleCount][3];
	int termRules[termRuleCount][2];
	int string[len];
	int literals[len];	

	//I represented S as an 8 to fit transBoth below. If adding more nonterminal rules, change.
	int utNonRules[8] = {812, 145, 15, 261, 113, 263, 26, 371};
	int utTermRules[36] = {40, 41, 52, 53, 54, 55, 56, 57, 58, 59, 
		510, 511, 512, 513, 514, 515, 516, 517, 518, 519, 620, 621, 
		622, 623, 624, 625, 626, 627, 628, 729, 730, 731, 732, 733, 734, 735};

	transBoth(utNonRules,nonRules,nonRuleCount,
		utTermRules,termRules,termRuleCount);
	
	// printf("transboth ended\n");
	// initialize manager
	SddLiteral var_count = term*len + nonTerm*len*(len+1)/2; // initial number of variables
	//printf("var_count = %ld\n", var_count);
	int auto_gc_and_minimize = 0; // disable (0) or enable (1) auto-gc & auto-min
	//SddManager* m = sdd_manager_create(var_count,auto_gc_and_minimize);
	Vtree* vtree = sdd_vtree_new(var_count, "right");
	SddManager* m = sdd_manager_new(vtree);

	//printf("sddparsings begin\n");
	//sdd_manager_auto_gc_and_minimize_on(m);
	SddNode* returned = sddParsings(m,nonTerm,term,nonRules,termRules,nonRuleCount,termRuleCount,start,len);
	//printf("sddparsings ended\n");

	FILE *countOut;
	countOut = fopen("testResults/countVsNonRule.txt","a");
	FILE *sizeOut;
	sizeOut = fopen("testResults/sizeVsNonRule.txt","a");

	//printf("file io ended\n");
	int size = sdd_size(returned);
	int nodeCount = sdd_manager_count(m);
	int modelCount = sdd_model_count(returned,m);
	
	printf("size: %d\n",size);
	printf("nodeCount: %d\n",nodeCount);
	printf("modelCount: %d\n",modelCount);	

	fprintf(countOut,"(%d,%d),",nonRuleCount,modelCount);
	fprintf(sizeOut,"(%d,%d),",nonRuleCount,size);
	
	sdd_save("../psdd/data/base/parsings.sdd",returned);
	sdd_vtree_save("../psdd/data/base/parsings.vtree",vtree);

	SddNode* tester;
	int i;
	
	// free manager
	sdd_manager_free(m);
	 
	return 0;
}