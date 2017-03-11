#include <stdio.h>
#include <stdlib.h>
#include "sddapi.h"

/*
	Rule representation:
		rules is an array of 3-tuples
		if rules[i][0] == -1:
			rule(i) is of the form X->x
			rules[i][1] = X
			rules[i][2] = x
		else
			rule(i) is of the form X->AB
			rules[i][0] = X
			rules[i][1] = A
			rules[i][2] = B
		PUT ALL NON-TERMINAL RULES BEFORE ALL TERMINAL RULES (terminal rules have higher indexes)
			
	String is an array of integers, corresponding to the input string (each integer in string must be < terminal)
			
	  0 1 2 3 4 5
	0 X O O O O O
	1 X X O O O O
	2 X X X O O O
	3 X X X X O O
	4 X X X X X O
	5 X X X X X X
*/

void printEverything(SddManager* m){
	int size = sdd_manager_size(m);
	int liveSize = sdd_manager_live_size(m);
	int deadSize = sdd_manager_dead_size(m);
	int count1 = sdd_manager_count(m);
	int liveCount = sdd_manager_live_count(m);
	int deadCount = sdd_manager_dead_count(m);
	int a = size;
	printf("size: %d\n",size);
	printf("liveSize: %d\n",liveSize);
	printf("deadSize: %d\n",deadSize);
	printf("count: %d\n",count1);
	printf("liveCount: %d\n",liveCount);
	printf("deadCount: %d\n\n",deadCount);
}


/*

*/

SddNode* cfgStringSdd(SddManager* m, int nonTerminal, int terminal, int rules[][3], int start, int ruleCount, int* string, int len){
	sdd_manager_auto_gc_and_minimize_off(m);
	SddNode* dy[len][len][nonTerminal];
	int i,j,k,index;
	
	//initialize everything to false
	for (i=0;i<len;i++) for (j=0;j<len;j++) for (k=0;k<nonTerminal;k++) dy[i][j][k] = sdd_manager_false(m);
	
	//set diagonal according to terminal rules
	for (index = 0;index < len; index++){
		for (i = 0; i < ruleCount; i++){
			if (rules[i][0] == -1 && rules[i][2] == string[index]){
				dy[index][index][rules[i][1]] = sdd_manager_true(m);
			}
		}
	}
	
	//this section builds a dynamic array according to number of parsings
	//build from top down(according to diagram above)
	for (j = 1; j < len; j++){
		//build from right to left
		for (i = j-1; i >= 0; i--){
			//check each non-terminal rule
			for (k = 0; rules[k][0] != -1; k++){
				for (index = 0; index+i < j; index++){
					if (dy[i][i+index][rules[k][1]] && dy[i+index+1][j][rules[k][2]]){
						dy[i][j][rules[k][0]] = sdd_disjoin(dy[i][j][rules[k][0]],sdd_conjoin(dy[i][i+index][rules[k][1]],dy[i+index+1][j][rules[k][2]],m),m);
					}
				}
			}
		}
	}
	
{	
	int size = sdd_size(dy[0][len-1][start]);
	printf("size: %d\n",size);
	
	dy[0][len-1][start] = sdd_minimize_cardinality(dy[0][len-1][start],m);
	
	size = sdd_size(dy[0][len-1][start]);
	printf("size: %d\n",size);
	
	sdd_manager_auto_gc_and_minimize_on(m);
	
	return dy[0][len-1][start];
}
}

/*
	# of literals = term*len + nonTerm*len*(len+1)/2
	first term*len literals:
		Terminal i corresponds to S(i/term,i%term)
	next nonTerm*len*(len+1)/2 correspond to non-terminals
	
	literal for T[i][j][k] = (j*(j+1)/2)*nonTerm+i*nonTerm+k + firstNon

*/
SddNode* sddParsings(SddManager*m, int nonTerm, int term, int nonRules[][3], int termRules[][2], int nonRuleCount, int termRuleCount, int start, int len){
	SddNode* S[term][len];
	int i,j,k,index;
	
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
			for(j = 0; j < nonTerm; j++){
				if (j != k){
					T[index][index][k] = sdd_conjoin(T[index][index][k],sdd_manager_literal(0-((index*(index+1)/2)*nonTerm+index*nonTerm+j + firstNon),m),m);
				}
			}
		}
	}
	
	return T[0][0][0];
	
}

SddNode* cfgWithRef(SddManager* m, Vtree* v, int nonTerminal, int terminal, int rules[][3], int start, int ruleCount, int* string, int len){
	SddNode* dy[len][len][nonTerminal];
	int i,j,k,index;
	
	//initialize everything to false
	for (i=0;i<len;i++) for (j=0;j<len;j++) for (k=0;k<nonTerminal;k++) dy[i][j][k] = sdd_manager_false(m);
	sdd_manager_auto_gc_and_minimize_on(m);
	
	//set diagonal according to terminal rules
	for (index = 0;index < len; index++){
		for (i = 0; i < ruleCount; i++){
			if (rules[i][0] == -1 && rules[i][2] == string[index]){
				dy[index][index][rules[i][1]] = sdd_manager_true(m);
			}
		}
	}
	
	//this section builds a dynamic array according to number of parsings
	//build from top down(according to diagram above)
	for (j = 1; j < len; j++){
		//build from right to left
		for (i = j-1; i >= 0; i--){
			//check each non-terminal rule
			for (k = 0; rules[k][0] != -1; k++){
				for (index = 0; index+i < j; index++){
					if (dy[i][i+index][rules[k][1]] && dy[i+index+1][j][rules[k][2]]){
						SddNode* tmp = sdd_apply_in_vtree(dy[i][i+index][rules[k][1]],dy[i+index+1][j][rules[k][2]],0,v,m);
						sdd_ref(tmp,m);
						dy[i][j][rules[k][0]] = sdd_apply_in_vtree(dy[i][j][rules[k][0]],tmp,1,v,m);
						sdd_ref(dy[i][j][rules[k][0]],m);
						sdd_deref(tmp,m);
					}
				}
			}
		}
	}
	
	
	//this section builds a dynamic array according to number of parsings
	//build from top down(according to diagram above)
	for (j = 1; j < len; j++){
		//build from right to left
		for (i = j-1; i >= 0; i--){
			//check each non-terminal rule
			for (k = 0; rules[k][0] != -1; k++){
				for (index = 0; index+i < j; index++){
					if (dy[i][i+index][rules[k][1]] && dy[i+index+1][j][rules[k][2]]){
						if (i != 0 || j != len-1 || rules[k][0] != start) sdd_deref(dy[i][j][rules[k][0]],m);
					}
				}
			}
		}
	}
	
	//int size = sdd_manager_size(m);
	int size = sdd_size(dy[0][len-1][start]);
	printf("size: %d\n",size);
	
	dy[0][len-1][start] = sdd_minimize_cardinality(dy[0][len-1][start],m);
	
	size = sdd_size(dy[0][len-1][start]);
	printf("size: %d\n",size);
	
	return dy[0][len-1][start];
}

int main(int argc, char** argv) {
	
//{	
	//*
	//0:A 1:B 2:S 3:T 4:U 5:X 6:Y
	int nonTerm = 7;
	int term = 2;
	int nonRuleCount = 6;
	int termRuleCount = 4;
	int start = 2;
	int len = 3;
	int nonRules[nonRuleCount][3];
	int termRules[termRuleCount][2];
	
	//S->XT
	nonRules[0][0] = 2;
	nonRules[0][1] = 5;
	nonRules[0][2] = 3;
	
	//S->UY
	nonRules[1][0] = 2;
	nonRules[1][1] = 4;
	nonRules[1][2] = 6;
	
	//X->AA
	nonRules[2][0] = 5;
	nonRules[2][1] = 0;
	nonRules[2][2] = 0;
	
	//Y->BB
	nonRules[3][0] = 6;
	nonRules[3][1] = 1;
	nonRules[3][2] = 1;
	
	//T->TB
	nonRules[4][0] = 3;
	nonRules[4][1] = 3;
	nonRules[4][2] = 1;
	
	//U->UA
	nonRules[5][0] = 4;
	nonRules[5][1] = 4;
	nonRules[5][2] = 0;
	
	//T->b
	termRules[0][0] = 3;
	termRules[0][1] = 1;
	
	//U->a
	termRules[1][0] = 4;
	termRules[1][1] = 0;
	
	//A->a
	termRules[2][0] = 0;
	termRules[2][1] = 0;

	//B->b
	termRules[3][0] = 1;
	termRules[3][1] = 1;


	//*/
//
	
	//0:A 1:B 2:T 3:S; 0:a 1:b
	
	//first example from text
/*	
	int nonTerm = 4;
	int term = 2;
	int nonRuleCount = 3;
	int termRuleCount = 2;
	int start = 3;
	int len = 4;
	int nonRules[nonRuleCount][3];
	int termRules[termRuleCount][2];
	
	//S->AT
	nonRules[0][0] = 3;
	nonRules[0][1] = 0;
	nonRules[0][2] = 2;
	
	//S->AB
	nonRules[1][0] = 3;
	nonRules[1][1] = 0;
	nonRules[1][2] = 1;
	
	//T->SB
	nonRules[2][0] = 2;
	nonRules[2][1] = 3;
	nonRules[2][2] = 1;
	
	//A->a
	termRules[0][0] = 0;
	termRules[0][1] = 0;
	
	//B->b
	termRules[1][0] = 1;
	termRules[1][1] = 1;
*/
	
/*
{	
	
	//*
	//0:A 1:B 2:S 3:T 4:U 5:X 6:Y
	int nonTerminal = 7;
	int terminal = 2;
	int ruleCount = 10;
	int start = 2;
	int len = 4;
	int rules[ruleCount][3];
	int string[len];
	string[0] = 0; string[1] = 0;
	int i;
	for(i = 2; i < len; i++) string[i] = 1; 
	

	//S->XT
	rules[0][0] = 2;
	rules[0][1] = 5;
	rules[0][2] = 3;
	
	//S->UY
	rules[1][0] = 2;
	rules[1][1] = 4;
	rules[1][2] = 6;
	
	//X->AA
	rules[2][0] = 5;
	rules[2][1] = 0;
	rules[2][2] = 0;
	
	//Y->BB
	rules[3][0] = 6;
	rules[3][1] = 1;
	rules[3][2] = 1;
	
	//T->TB
	rules[4][0] = 3;
	rules[4][1] = 3;
	rules[4][2] = 1;
	
	//U->UA
	rules[5][0] = 4;
	rules[5][1] = 4;
	rules[5][2] = 0;

	//T->b
	rules[6][0] = -1;
	rules[6][1] = 3;
	rules[6][2] = 1;
	
	//U->a
	rules[7][0] = -1;
	rules[7][1] = 4;
	rules[7][2] = 0;
	
	//A->a
	rules[8][0] = -1;
	rules[8][1] = 0;
	rules[8][2] = 0;

	//B->b
	rules[9][0] = -1;
	rules[9][1] = 1;
	rules[9][2] = 1;
	///
	
	//0:A 1:B 2:T 3:S; 0:a 1:b
	/*
	//first example from text
	
	int nonTerminal = 4;
	int terminal = 2;
	int ruleCount = 5;
	int start = 3;
	int len = 6;
	int rules[ruleCount][3];
	int string[6] = {0,0,0,1,1,1};
	
	//S->AT
	rules[0][0] = 3;
	rules[0][1] = 0;
	rules[0][2] = 2;
	
	//S->AB
	rules[1][0] = 3;
	rules[1][1] = 0;
	rules[1][2] = 1;
	
	//T->SB
	rules[2][0] = 2;
	rules[2][1] = 3;
	rules[2][2] = 1;
	
	//A->a
	rules[3][0] = -1;
	rules[3][1] = 0;
	rules[3][2] = 0;
	
	//B->b
	rules[4][0] = -1;
	rules[4][1] = 1;
	rules[4][2] = 1;
	///
//}
*/

	// initialize manager
	SddLiteral var_count = term*len + nonTerm*len*(len+1)/2; // initial number of variables
	int auto_gc_and_minimize = 0; // disable (0) or enable (1) auto-gc & auto-min
	SddManager* m = sdd_manager_create(var_count,auto_gc_and_minimize);
	
	//Vtree* vtree = sdd_vtree_new(var_count, "right");
	//SddManager* m = sdd_manager_new(vtree);

	//SddNode* returned = cfgStringSdd(m,nonTerminal,terminal,rules,start,ruleCount,string,len);
	//SddNode* returned = cfgWithRef(m,vtree,nonTerminal,terminal,rules,start,ruleCount,string,len);
	SddNode* returned = sddParsings(m,nonTerm,term,nonRules,termRules,nonRuleCount,termRuleCount,start,len);
	
	returned = sdd_condition(-1,returned,m);
	int modelCount = sdd_model_count(returned,m);
	printf("count: %d\n",modelCount);
	
	sdd_save_as_dot("cfgSdd.dot",returned);
	
	
	/*
	int size = sdd_size(returned);
	printf("size: %d\n",size);
	
	if (returned == sdd_manager_false(m)){
		printf("not valid\n");
	}
	else printf("valid\n");
	
	if (returned == sdd_manager_true(m)){
		printf("true\n");
	}
	else printf("not true\n");
	
	sdd_save_as_dot("cfgSdd.dot",returned);
	*/
	
	// free manager
	sdd_manager_free(m);
	// free vtree
	//sdd_vtree_free(vtree);
	 
	return 0;
}