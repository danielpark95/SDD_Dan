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
		PUT ALL NON-TERMINAL RULES BEFORE ALL TERMINAL RULES
			
	String is an array of integers, corresponding to the input string (each integer in string must be < terminal)
			
	  0 1 2 3 4 5
	0 X O O O O O
	1 X X O O O O
	2 X X X O O O
	3 X X X X O O
	4 X X X X X O
	5 X X X X X X
*/

int cky(int nonTerminal, int terminal, int rules[][3], int start, int ruleCount, int* string, int len){
	int dy[len][len][nonTerminal];
	int i,j,k,index;
	
	//set everything to 0 initially
	for (i=0;i<len;i++) for (j=0;j<len;j++) for (k=0;k<nonTerminal;k++) dy[i][j][k] = 0;
	
	//set diagonals according to terminal rules
	for (index = 0;index < len; index++){
		for (i = 0; i < ruleCount; i++){
			if (rules[i][0] == -1 && rules[i][2] == string[index]){
				dy[index][index][rules[i][1]] = 1;
				//printf("%d,%d,%d\n",index,index,rules[i][1]);
			}
		}
	}
	
	/*
	//this section builds a dynamic array with no concept of multiple parsings
	//build from top down(according to diagram above)
	for (j = 1; j < len; j++){
		//build from right to left
		for (i = j-1; i >= 0; i--){
			//check each non-terminal rule
			for (k = 0; rules[k][0] != -1; k++){
				//difference between i & j is the number of ways to break down string
				for (index = 0; index+i < j; index++){
					if (dy[i][i+index][rules[k][1]] && dy[i+index+1][j][rules[k][2]]){
						printf("%d,%d,%d\n",i,j,rules[k][0]);
						dy[i][j][rules[k][0]] = 1;
					}
				}
			}
		}
	}
	int ableToParse = dy[0][len-1][start];
	*/
	
	int numWays[nonTerminal];
	
	//this section builds a dynamic array according to number of parsings
	//build from top down(according to diagram above)
	for (j = 1; j < len; j++){
		//build from right to left
		for (i = j-1; i >= 0; i--){
			
			//0 out numWays
			//numWays[k] will keep track of how many ways we can get nonTerminal k at position (i,j)
			for(k=0;k<nonTerminal;k++){
				numWays[k]=0;
			}
			
			//check each non-terminal rule
			for (k = 0; rules[k][0] != -1; k++){
				for (index = 0; index+i < j; index++){
					if (dy[i][i+index][rules[k][1]] && dy[i+index+1][j][rules[k][2]]){
						numWays[rules[k][0]] += dy[i][i+index][rules[k][1]]*dy[i+index+1][j][rules[k][2]];
						//printf("numWays: %d\n",numWays[rules[k][0]]);
					}
				}
			}
			//plug numWays values into dy
			for(k=0;k<nonTerminal;k++){
				dy[i][j][k] = numWays[k];
			}
		}
	}
	
	
	return dy[0][len-1][start];
}

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
	*/
	//*
	//0:A 1:B 2:S 3:T 4:U 5:X 6:Y 7:M 8:N 9:O 10:P
	int nonTerminal = 11;
	int terminal = 2;
	int ruleCount = 18;
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
	
	/*
	//ADDED ONE
	//T->TT
	rules[6][0] = 3;
	rules[6][1] = 3;
	rules[6][2] = 3;
	*/
	
	//X->MM
	rules[6][0] = 5;
	rules[6][1] = 7;
	rules[6][2] = 7;
	
	//T->TN
	rules[7][0] = 3;
	rules[7][1] = 3;
	rules[7][2] = 8;
	
	//X->OO
	rules[8][0] = 5;
	rules[8][1] = 9;
	rules[8][2] = 9;
	
	//T->TP
	rules[9][0] = 3;
	rules[9][1] = 3;
	rules[9][2] = 10;
	
	//T->b
	rules[10][0] = -1;
	rules[10][1] = 3;
	rules[10][2] = 1;
	
	//U->a
	rules[11][0] = -1;
	rules[11][1] = 4;
	rules[11][2] = 0;
	
	//A->a
	rules[12][0] = -1;
	rules[12][1] = 0;
	rules[12][2] = 0;

	//B->b
	rules[13][0] = -1;
	rules[13][1] = 1;
	rules[13][2] = 1;

	//M->a
	rules[14][0] = -1;
	rules[14][1] = 7;
	rules[14][2] = 0;

	//N->b
	rules[15][0] = -1;
	rules[15][1] = 8;
	rules[15][2] = 1;

	//O->a
	rules[16][0] = -1;
	rules[16][1] = 9;
	rules[16][2] = 0;

	//P->b
	rules[17][0] = -1;
	rules[17][1] = 10;
	rules[17][2] = 1;
	
	//*/
	
	//int returned = cky(nonTerminal,terminal,rules,start,ruleCount,string,len);
	int returned = ckyParseCount(nonTerminal,terminal,rules,start,ruleCount,string,len);
	printf("returned %d\n",returned);
}