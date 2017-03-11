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
	0 AJKL
	1 MNOP ARST
	2 UVWX YZS8 9B12
	3 34S6 789T 1234 5B78
	  ABST
	
	
	
	int nonTerm = 7;
	int term = 2;
	int nonRuleCount = 6;
	int termRuleCount = 4;
	int start = 2;
	int len = 3;
	
	AB
	CD
	EF
	
	GHIJKLM
	NOPQRST UVWXYZ7
	8901234 5678901 2345678
	ABSTUXY
	0123456
	
	12
	23
	56
	
	7890123 10
	4567890 1234567 20
	8901234 5678901 2345678 30 40
	
	12
	23
	56
	78
	
	9012345 10
	6789012 3456789 20
	0123456 7890123 4567890 30 40 50
	1234567 8901234 5678901 2345678 60 70
	ABSTUXY
	
	
	int nonTerm = 3;
	int term = 2;
	int nonRuleCount = 3;
	int termRuleCount = 2;
	int start = 2;
	int len = 3;
	
	AB
	CD
	EF
	
	GHI
	JKL MNO
	PQR STU VWX
	
	aab
	
	A
	-A
	SSB
	
*/



SddNode* sddParsings(SddManager*m, int nonTerm, int term, int nonRules[][3], int termRules[][2], int nonRuleCount, int termRuleCount, int start, int len){
	SddNode* S[term][len];
	int i,j,k,index;
	
	printf("hi\n");
	
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
	
	T[14][14][1] = sdd_manager_true(m);
	printf("here\n");
	
	//set diagonal according to terminal rules
	for (index = 0;index < len; index++){
		for(k = 0; k < nonTerm; k++){
			T[index][index][k] = sdd_manager_false(m);
		}	
		for(i = 0; i < termRuleCount; i++){
			k = termRules[i][0];
			int termVal = termRules[i][1];
			T[index][index][k] = sdd_disjoin(T[index][index][k],S[termVal][index],m);
		}
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
	
	for (j = 1; j < len; j++){
		for (i = j-1; i >= 0; i--){
			empties[i][j] = sdd_manager_true(m);
			for(k = 0; k < nonTerm; k++){
				empties[i][j] = sdd_conjoin(empties[i][j],sdd_manager_literal(0-((j*(j+1)/2)*nonTerm+i*nonTerm+k + firstNon),m),m);
			}
		}
	}
	
	
	
	for (j = 1; j < len; j++){
		printf("j:%d\n",j);
		for (i = j-1; i >= 0; i--){
			printf("i:%d\n",i);
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
					//printf("up,east: %d%d\n",up,east);
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
				//printf("literal: %d\n",(j*(j+1)/2)*nonTerm+i*nonTerm+k + firstNon);
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


void stringToLiterals(int string[],int literals[],int len,int term){
	int index;
	for (index = 0;index < len;index++){
		literals[index] = index*term+string[index]+1;
	}
}

int main(int argc, char** argv) {
	
	//*
	//*
	int nonTerm = 3;
	int term = 2;
	int nonRuleCount = 3;
	int termRuleCount = 2;
	int start = 2;
	int len = 16;
	int nonRules[nonRuleCount][3];
	int termRules[termRuleCount][2];
	int string[len];
	//string[0] = 0; string[1] = 0; string[2] = 0; string[3] = 0;
	int literals[len];	
	
	
	//0:A 1:B 2:S, 0:a 1:b 2:x
	
	//S -> AS
	nonRules[0][0] = 2;
	nonRules[0][1] = 0;
	nonRules[0][2] = 2;
	
	//S -> SB
	nonRules[1][0] = 2;
	nonRules[1][1] = 2;
	nonRules[1][2] = 1;
	
	//S -> AB
	nonRules[2][0] = 2;
	nonRules[2][1] = 0;
	nonRules[2][2] = 1;
	
	//A -> a
	termRules[0][0] = 0;
	termRules[0][1] = 0;
	
	//B -> b
	termRules[1][0] = 1;
	termRules[1][1] = 1;
	
	//A -> x
	termRules[2][0] = 0;
	termRules[2][1] = 2;

	// initialize manager
	SddLiteral var_count = term*len + nonTerm*len*(len+1)/2; // initial number of variables
	int auto_gc_and_minimize = 0; // disable (0) or enable (1) auto-gc & auto-min
	//SddManager* m = sdd_manager_create(var_count,auto_gc_and_minimize);
	Vtree* vtree = sdd_vtree_new(var_count, "right");
	SddManager* m = sdd_manager_new(vtree);
	
	
	SddNode* returned = sddParsings(m,nonTerm,term,nonRules,termRules,nonRuleCount,termRuleCount,start,len);


	int modelCount = sdd_model_count(returned,m);
	printf("count: %d\n",modelCount);
	int size = sdd_size(returned);
	printf("size %d\n",size);
	
	
	// free manager
	sdd_manager_free(m);
	 
	return 0;
}