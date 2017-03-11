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

int ckyParseCount(int nonTerminal, int terminal, int rules[][3], int start, int ruleCount, int* string, int len){
	int dy[len][len][nonTerminal];
	int i,j,k,index;
	
	//initialize everything to 0
	for (i=0;i<len;i++) for (j=0;j<len;j++) for (k=0;k<nonTerminal;k++) dy[i][j][k] = 0;
	
	//set diagonals according to terminal rules
	for (index = 0;index < len; index++){
		for (i = 0; i < ruleCount; i++){
			if (rules[i][0] == -1 && rules[i][2] == string[index]){
				dy[index][index][rules[i][1]] = 1;
			}
		}
	}

	//int numWays[nonTerminal];
	
	//this section builds a dynamic array according to number of parsings
	//build from top down(according to diagram above)
	for (j = 1; j < len; j++){
		//build from right to left
		for (i = j-1; i >= 0; i--){
			//0 out numWays
			//numWays[k] will keep track of how many ways we can get nonTerminal k at position (i,j)
			//for(k=0;k<nonTerminal;k++){
			//	numWays[k]=0;
			//}
			
			//check each non-terminal rule
			for (k = 0; rules[k][0] != -1; k++){
				for (index = 0; index+i < j; index++){
					if (dy[i][i+index][rules[k][1]] && dy[i+index+1][j][rules[k][2]]){
						//numWays[rules[k][0]] += dy[i][i+index][rules[k][1]]*dy[i+index+1][j][rules[k][2]];
						dy[i][j][rules[k][0]] += dy[i][i+index][rules[k][1]]*dy[i+index+1][j][rules[k][2]];
					}
				}
			}
			//plug numWays values into dy
			//for(k=0;k<nonTerminal;k++){
			//	dy[i][j][k] = numWays[k];
			//}
		}
	}
	
	return dy[0][len-1][start];
}

int main(int argc, char** argv) {
	
	//0:A 1:B 2:S 3:T 4:U 5:X 6:Y
	int nonTerminal = 7;
	int terminal = 2;
	int ruleCount = 10;
	int start = 2;
	int len = 4;
	int rules[ruleCount][3];
	int string[4] = {0,0,1,1};
	
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
	
	int returned = ckyParseCount(nonTerminal,terminal,rules,start,ruleCount,string,len);
	printf("returned %d\n",returned);
}