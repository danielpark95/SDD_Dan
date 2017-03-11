#include <stdio.h>
#include <stdlib.h>
#include "sddapi.h"

/*
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
			
	String is an array of integers, corresponding to the input string (each integer in string must be < terminal)
			
	  0 1 2 3 4 5
	0 X O O O O O
	1 X X O O O O
	2 X X X O O O
	3 X X X X O O
	4 X X X X X O
	5 X X X X X X
	
	# of literals = term*len + nonTerm*len*(len+1)/2
	first term*len literals:
		Terminal i corresponds to S(i/term,i%term)
	next nonTerm*len*(len+1)/2 correspond to non-terminals

	literal for T[i][j][k] = (j*(j+1)/2)*nonTerm+i*nonTerm+k + firstNon
	
	int nonTerm = 4;
	int term = 2;
	int nonRuleCount = 3;
	int termRuleCount = 2;
	int start = 3;
	int len = 4;
	
	AB
	CD
	EF
	GH
	   0    1    2    3
	0 IJKL
	1 MNOP QRST
	2 UVWX YZ78 9012
	3 3456 7890 1234 5678
	  ABST
	
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
			//printf("literalX %d\n",(index*(index+1)/2)*nonTerm+index*nonTerm+k + firstNon);
			for(j = 0; j < nonTerm; j++){
				if (j != k){
					T[index][index][k] = sdd_conjoin(T[index][index][k],sdd_manager_literal(0-((index*(index+1)/2)*nonTerm+index*nonTerm+j + firstNon),m),m);
				}
			}
		}
	}
	
	//printf("firstNon %d\n",firstNon);
	
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
	
	//printf("a\n");
	
	for (j = 1; j < len; j++){
		for (i = j-1; i >= 0; i--){
			for(k = 0; k < nonTerm; k++){
				T[i][j][k] = sdd_manager_false(m);
			}
			//printf("b\n");
			for(index = 0; index < nonRuleCount; index++){
				int root = nonRules[index][0];
				int left = nonRules[index][1];
				int right = nonRules[index][2];
				//printf("root:%d left:%d right:%d\n",root,left,right);
				int up;
				int east = i+1;
				for(up = i; up < j; up++){
					//printf("c\n");
					//printf("i %d up %d left %d east %d j %d right %d\n",i,up,left,east,j,right);
					//printf("up,east: %d%d\n",up,east);
					SddNode* delta = sdd_conjoin(T[i][up][left],T[east][j][right],m);
					//printf("c.5\n");
					int midup, mideast, counter;
					for(midup = up+1;midup <= j;midup++){
						//printf("d\n");
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
			//printf("e\n");
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

//for rules in the form for generateDataset
void translateRules(int nonRules[][3],int termRules[][2],int nonTerm){
	int maxRuleCount = 2;
	double rules[nonTerm][maxRuleCount+1][3];
	
	//0:A 1:B 2:S 3:T 4:U 5:V, 0:a 1:b
	
	//A -> a 1
	rules[0][0][0] = 1;
	rules[0][0][1] = -1;
	rules[0][0][2] = 0;
	
	//Do this to signify that we have finished all rules for a given nonTerm
	rules[0][1][0] = -1;
	
	//B -> b 1
	rules[1][0][0] = 1;
	rules[1][0][1] = -1;
	rules[1][0][2] = 1;
	
	rules[1][1][0] = -1;
	
	//S -> AT 1
	rules[2][0][0] = 1;
	rules[2][0][1] = 0;
	rules[2][0][2] = 3;
	
	rules[2][1][0] = -1;
	
	//T -> AU 0.9
	rules[3][0][0] = 0.9;
	rules[3][0][1] = 0;
	rules[3][0][2] = 4;
	
	//T -> BU 0.1
	rules[3][1][0] = 0.1;
	rules[3][1][1] = 1;
	rules[3][1][2] = 4;
	
	rules[3][2][0] = -1;
	
	//U -> AV 1
	rules[4][0][0] = 1;
	rules[4][0][1] = 0;
	rules[4][0][2] = 5;
	
	rules[4][1][0] = -1;
	
	//V -> AA 0.7
	rules[5][0][0] = 0.7;
	rules[5][0][1] = 0;
	rules[5][0][2] = 0;
	
	//V -> BA 0.3
	rules[5][1][0] = 0.3;
	rules[5][1][1] = 1;
	rules[5][1][2] = 0;
	
	rules[5][2][0] = -1;
	
	/*
	int maxRuleCount = 3;
	
	//A -> a 0.4
	rules[0][0][0] = 0.4;
	rules[0][0][1] = -1;
	rules[0][0][2] = 0;
	
	//A -> x 0.4
	rules[0][1][0] = 0.4;
	rules[0][1][1] = -1;
	rules[0][1][2] = 2;
	
	//A -> AA 0.2
	rules[0][2][0] = 0.2;
	rules[0][2][1] = 0;
	rules[0][2][2] = 0;
	
	//Do this to signify that we have finished all rules for a given nonTerm
	rules[0][3][0] = -1;
	
	//B -> b 0.9
	rules[1][0][0] = 0.9;
	rules[1][0][1] = -1;
	rules[1][0][2] = 1;
	
	//B -> BB 0.1
	rules[1][1][0] = 0.1;
	rules[1][1][1] = 1;
	rules[1][1][2] = 1;
	
	rules[1][2][0] = -1;
	
	//S -> AS 0.9
	rules[2][0][0] = 0.9;
	rules[2][0][1] = 0;
	rules[2][0][2] = 2;
	
	//S -> AB 0.05
	rules[2][1][0] = 0.05;
	rules[2][1][1] = 0;
	rules[2][1][2] = 1;
	
	//S -> y 0.05
	rules[2][2][0] = 0.05;
	rules[2][2][1] = -1;
	rules[2][2][2] = 3;
	
	rules[2][3][0] = -1;
	*/
	
	//the index of the nex terminal rule to set for the outgoing termRules array
	int termRuleOut = 0;
	
	//the index of the next rule to translate from the rules array
	int nonRule = 0;
	
	//the non terminal we are currently evaluating from the rules array
	int nonTermNum = 0;
	
	//the index of the nex non terminal rule to set for the outgoing nonRules array
	int nonRuleOut = 0;
	
	while(nonTermNum < nonTerm){
		//we have finished all the rules for that non terminal
		if (rules[nonTermNum][nonRule][0] == -1){
			nonTermNum++;
			nonRule = 0;
			if (nonTermNum >= nonTerm) break;
		}
		
		//terminal rule
		if (rules[nonTermNum][nonRule][1] == -1){
			termRules[termRuleOut][0] = nonTermNum;
			termRules[termRuleOut][1] = rules[nonTermNum][nonRule][2];
			//printf("term rule %d: %d -> %d\n",termRuleOut,termRules[termRuleOut][0],termRules[termRuleOut][1]);
			termRuleOut++;
			nonRule++;
		}
		
		//non terminal rule
		else{
			nonRules[nonRuleOut][0] = nonTermNum;
			nonRules[nonRuleOut][1] = rules[nonTermNum][nonRule][1];
			nonRules[nonRuleOut][2] = rules[nonTermNum][nonRule][2];
			//printf("non term rule %d: %d -> %d %d\n",nonRuleOut,nonRules[nonRuleOut][0],nonRules[nonRuleOut][1],nonRules[nonRuleOut][2]);
			nonRuleOut++;
			nonRule++;
		}
	}
}

//utn is a set of non terminal rules of the form "121" which translates to 1 -> 2 1, utt is a set of terminal rules of the form "21" which translates to 2 -> 1
//if the non-terminal on the left is 0, omit it -> write 022 as just 22
void transBoth(int utn[],int nonRules[][3],int nonRuleCount,int utt[],int termRules[][2],int termRuleCount){
	int i;
	for (i=0;i<nonRuleCount;i++){
		int firstR = utn[i]%100;
		nonRules[i][0] = utn[i]/100;
		nonRules[i][1] = firstR/10;
		nonRules[i][2] = firstR%10;
	}
	for (i=0;i<termRuleCount;i++){
		termRules[i][0] = utt[i]/10;
		termRules[i][1] = utt[i]%10;
	}
}

void stringToLiterals(int string[],int literals[],int len,int term){
	int index;
	for (index = 0;index < len;index++){
		literals[index] = index*term+string[index]+1;
	}
}

int main(int argc, char** argv) {
	
	int tmpNonCt = 20;
	int tmpTermCt = 6;
	int nonTerm = 6;
	int term = 5;
	//int nonRuleCount = atoi(argv[1]);
	int nonRuleCount = 5;
	int termRuleCount = 6;
	int start = 2;
	int len = 6;
	//int nonRules[nonRuleCount][3];
	int nonRules[tmpNonCt][3];
	//int termRules[termRuleCount][2];
	int termRules[tmpTermCt][2];
	int string[len];
	//string[0] = 0; string[1] = 0; string[2] = 0; string[3] = 0;
	int literals[len];	
	
	//stringToLiterals(string,literals,len,term);
	//translateRules(nonRules,termRules,nonTerm);
	
	//0:A 1:B 2:S 3:M 4:N 5:X | 0:a 1:b 2:x 3:m 4:n
	int utNonRules[20] = {000,111,202,221,201,232,235,222,555,22,43,251,230,333,321,444,522,301,353,1};
	//int utTermRules[6] = {00,11,52,32,42,01,02,12,10,41,40,51,50,31,30,20,21,22};
	int utTermRules[6] = {00,11,22,33,44,52};
	transBoth(utNonRules,nonRules,nonRuleCount,utTermRules,termRules,termRuleCount);
	
	// initialize manager
	SddLiteral var_count = term*len + nonTerm*len*(len+1)/2; // initial number of variables
	//printf("var_count = %ld\n", var_count);
	int auto_gc_and_minimize = 0; // disable (0) or enable (1) auto-gc & auto-min
	//SddManager* m = sdd_manager_create(var_count,auto_gc_and_minimize);
	Vtree* vtree = sdd_vtree_new(var_count, "right");
	SddManager* m = sdd_manager_new(vtree);
	
	//sdd_manager_auto_gc_and_minimize_on(m);
	SddNode* returned = sddParsings(m,nonTerm,term,nonRules,termRules,nonRuleCount,termRuleCount,start,len);
	
	FILE *countOut;
	countOut = fopen("testResults/countVsNonRule4.txt","a");
	FILE *sizeOut;
	sizeOut = fopen("testResults/sizeVsNonRule4.txt","a");
	
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