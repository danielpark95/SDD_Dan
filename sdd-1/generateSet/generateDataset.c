#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int stack[1000];

int *sp;

#define push(sp, n) (*((sp)++) = (n))
#define pop(sp) (*--(sp))

//returns the index of the rule that is chosen.
int selectRule(double rules[][3]){
	int r = rand() % 100;
	//printf ("%d\n",r);
	int index;
	int total = 0;
	for(index = 0; total < 100; index++){
		total += rules[index][0] * 100;
		if (r < total){
			//printf("selected: %d\n",index);
			return index;
		}
	}
}


// This function takes in a set of rules, a word length, and a set size and places into words setSize words, each of length len
// rules are in CNF and have a weight associated (0-100)
// ex: A -> BC with probability 0.4, or A -> a with probability 0.6
//	rules[x]: x is the non terminal (A)
//  rules[][y]: y is the index of the specific rule 
//	rules[][][0]: probability (0.4)
//	rules[][][1]: if non terminal rule: first non terminal (B)
//			      else: -1
//	rules[][][2]: terminal or second non terminal
//	len must be >= 2
void createSet(int len, int maxRuleCount, int nonTerm, double rules[nonTerm][maxRuleCount][3], int setSize, int words[setSize][len], int term, int start){
	//stack is empty iff sp == stack
	sp = stack;
	int wordNum,index,success,selected,top;
	index = 0;
	
	for(wordNum = 0;wordNum < setSize;wordNum++){
		int runs = 0;
		
		//outer loop starts creating a word at beginning and after any failure
		while(1){
			success = 0;
			index = 0;
			sp = stack;
			selected = selectRule(rules[start]);
			
			//only start with nonTerminal rules
			if (rules[start][selected][1] == -1) continue;
			
			//want to evaluate left nonTerminal first
			push(sp,rules[start][selected][2]);
			//printf("pushed: %g\n",rules[start][selected][2]);
			push(sp,rules[start][selected][1]);
			//printf("pushed: %g\n",rules[start][selected][1]);
			
			//inner loop tries to create a new word
			//if the word is the correct length, keep it
			//else, discard and return to top of outer loop
			while(index < len){
				if (sp == stack) break;
				top = pop(sp);
				//printf("popped: %d\n",top);
				selected = selectRule(rules[top]);
				
				//non terminal rule
				if (rules[top][selected][1] == -1){
					words[wordNum][index] = rules[top][selected][2];
					index++;
					if (sp == stack){
						if (index == len) success = 1;
						break;
					}
				}
				else {
					push(sp,rules[top][selected][2]);
					//printf("pushed: %g\n",rules[start][selected][2]);
					push(sp,rules[top][selected][1]);
					//printf("pushed: %g\n",rules[start][selected][1]);
				}
			}
			
			if (success == 1) break;
			runs++;
		}
	}
}

//len is maxLen
void createRandom(int len, int maxRuleCount, int nonTerm, double rules[nonTerm][maxRuleCount][3], int setSize, int words[setSize][len], int term, int start, int strCt[]){
	//stack is empty iff sp == stack
	sp = stack;
	int wordNum,index,success,selected,top,count;
	index = 0;
	
	
	
	for(wordNum = 0;wordNum < setSize;wordNum++){
		//printf("wordNum: %d\n",wordNum);
		//outer loop starts creating a word at beginning and after any failure
		while(1){
			//printf("in\n");
			success = 0;
			index = 0;
			sp = stack;
			selected = selectRule(rules[start]);
			
			if (rules[start][selected][1] == -1){
				words[wordNum][0] = rules[start][selected][2];
				words[wordNum][1] = -1;
				strCt[1]++;
				break;
			}
			
			//want to evaluate left nonTerminal first
			push(sp,rules[start][selected][2]);
			push(sp,rules[start][selected][1]);
			
			//inner loop tries to create a new word
			//if the word is the correct length, keep it
			//else, discard and return to top of outer loop
			while(index < len){
				if (sp == stack){
					success = 1;
					//index is currently set to wordLen
					words[wordNum][index] = -1;
					//printf("index: %d, strCt[index]: %d\n",index,strCt[index]);
					strCt[index]++;
					break;
				} 
				top = pop(sp);
				selected = selectRule(rules[top]);
				
				//non terminal rule
				if (rules[top][selected][1] == -1){
					words[wordNum][index] = rules[top][selected][2];
					index++;
				}
				else {
					push(sp,rules[top][selected][2]);
					push(sp,rules[top][selected][1]);
				}
			}
			
			if (success == 1) break;
		}
	}
}


int main(int argc, char *argv[]){
	srand(time(NULL));
	int r = rand() % 50;
	int i,j,k,l;
	
	int len = 5;
	int nonTerm = 6;
	//max rules any given nonTerminal has
	int maxRuleCount = 2;
	double rules[nonTerm][maxRuleCount][3];
	int setSize = strtol(argv[2], NULL, 0);
	//int setSize = 1000;
	int words[setSize][len];
	int term = 2;
	int start = 2;
	
	/*
	//0:A 1:B 2:S, 0:a 1:b 2:x 3:y
	
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
	
	//B -> b 0.6
	rules[1][0][0] = 0.6;
	rules[1][0][1] = -1;
	rules[1][0][2] = 1;
	
	//B -> BB 0.4
	rules[1][1][0] = 0.4;
	rules[1][1][1] = 1;
	rules[1][1][2] = 1;
	
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
	//*/
	
	//0:A 1:B 2:S 3:T 4:U 5:V, 0:a 1:b
	
	//A -> a 1
	rules[0][0][0] = 1;
	rules[0][0][1] = -1;
	rules[0][0][2] = 0;
	
	//B -> b 1
	rules[1][0][0] = 1;
	rules[1][0][1] = -1;
	rules[1][0][2] = 1;
	
	//S -> AT 1
	rules[2][0][0] = 1;
	rules[2][0][1] = 0;
	rules[2][0][2] = 3;
	
	//T -> AU 0.9
	rules[3][0][0] = 0.9;
	rules[3][0][1] = 0;
	rules[3][0][2] = 4;
	
	//T -> BU 0.1
	rules[3][1][0] = 0.1;
	rules[3][1][1] = 1;
	rules[3][1][2] = 4;
	
	//U -> AV 1
	rules[4][0][0] = 1;
	rules[4][0][1] = 0;
	rules[4][0][2] = 5;
	
	//V -> AA 0.7
	rules[5][0][0] = 0.7;
	rules[5][0][1] = 0;
	rules[5][0][2] = 0;
	
	//V -> BA 0.3
	rules[5][1][0] = 0.3;
	rules[5][1][1] = 1;
	rules[5][1][2] = 0;
	
	int strCt[len+1];
	for(i = 0; i < len+1; i++){
		strCt[i] = 0;
	}
	/*
	
	//0:A 1:B 2:S 3:T 4:U 5:V, 0:a 1:b 2:x 3:y
	
	//A -> a 0.8
	rules[0][0][0] = 0.8;
	rules[0][0][1] = -1;
	rules[0][0][2] = 0;
	
	//A -> AA 0.2
	rules[0][1][0] = 0.2;
	rules[0][1][1] = 0;
	rules[0][1][2] = 0;
	
	//B -> b 0.8
	rules[1][0][0] = 0.8;
	rules[1][0][1] = -1;
	rules[1][0][2] = 1;
	
	//B -> VB 0.1
	rules[1][1][0] = 0.1;
	rules[1][1][1] = 5;
	rules[1][1][2] = 1;
	
	//B -> AV 0.1
	rules[1][2][0] = 0.1;
	rules[1][2][1] = 0;
	rules[1][2][2] = 5;
	
	//S -> AT 0.5
	rules[2][0][0] = 0.5;
	rules[2][0][1] = 0;
	rules[2][0][2] = 3;
	
	//S -> US 0.3
	rules[2][1][0] = 0.3;
	rules[2][1][1] = 4;
	rules[2][1][2] = 2;
	
	//S -> y 0.2
	rules[2][2][0] = 0.2;
	rules[2][2][1] = -1;
	rules[2][2][2] = 3;
	
	//T -> AU 0.6
	rules[3][0][0] = 0.6;
	rules[3][0][1] = 0;
	rules[3][0][2] = 4;
	
	//T -> BU 0.1
	rules[3][1][0] = 0.1;
	rules[3][1][1] = 1;
	rules[3][1][2] = 4;
	
	//T -> b 0.3
	rules[3][2][0] = 0.3;
	rules[3][2][1] = -1;
	rules[3][2][2] = 1;
	
	//U -> AV 0.6
	rules[4][0][0] = 0.6;
	rules[4][0][1] = 0;
	rules[4][0][2] = 5;
	
	//U -> y 0.1
	rules[4][1][0] = 0.1;
	rules[4][1][1] = -1;
	rules[4][1][2] = 3;
	
	//U -> x 0.3
	rules[4][2][0] = 0.3;
	rules[4][2][1] = -1;
	rules[4][2][2] = 2;
	
	//V -> AU 0.4
	rules[5][0][0] = 0.4;
	rules[5][0][1] = 0;
	rules[5][0][2] = 4;
	
	//V -> BA 0.3
	rules[5][1][0] = 0.3;
	rules[5][1][1] = 1;
	rules[5][1][2] = 0;
	
	//V -> y 0.3
	rules[5][2][0] = 0.3;
	rules[5][2][1] = -1;
	rules[5][2][2] = 3;
	*/
	//printf("%d\n",selectRule(rules[2]));
	
	int runNum = 0;
	int totalRuns = strtol(argv[1], NULL, 0);
	
	for(runNum = 0;runNum < totalRuns; runNum++){
		
		
		createSet(len,maxRuleCount,nonTerm,rules,setSize,words,term,start);
		//createRandom(len,maxRuleCount,nonTerm,rules,setSize,words,term,start,strCt);

		/*
		for(i=0;i<=len;i++){
			printf("%d\n",strCt[i]);
			//printf("%d: %d ",i,strCt[i]);
			//if (i%10 == 9) printf("\n");
		}
		*/
		char pcfgTrainBuf[32];
		snprintf(pcfgTrainBuf, sizeof(char) * 32, "./pcfgTraining%i.txt", runNum);
		char psddTrainBuf[32];
		snprintf(psddTrainBuf, sizeof(char) * 32, "./psddTraining%i.txt", runNum);

		FILE *pcfgTrain;
		pcfgTrain = fopen(pcfgTrainBuf, "w");
		FILE *psddTrain;
		psddTrain = fopen(psddTrainBuf,"w");

		/*
		for (i = 0; i < setSize; i++){
			for (j = 0; j < len; j++){
				fprintf(pcfgTrain,"%d",words[i][j]);
				if (j != len-1) fprintf(pcfgTrain,",");
			}
			fprintf(pcfgTrain,"\n");
		}
		*/
		

		for(i = 0; i < setSize; i++){
			for(j = 0; j < len; j++){
				int theWord = words[i][j];
				int done = 0;
				switch (theWord)
				{
					case 0:
						fprintf(pcfgTrain,"a");
						break;
					case 1:
						fprintf(pcfgTrain,"b");
						break;
					case 2:
						fprintf(pcfgTrain,"x");
						break;
					case 3:
						fprintf(pcfgTrain,"y");
						break;		
				}
				if (j != len-1) fprintf(pcfgTrain," ");
			}
			fprintf(pcfgTrain,"\n");
		}
		

		//this code translates a set of words into an SDD representation
		for (i = 0; i < setSize; i++){
			for (j = 0; j < len; j++){
				l = 0;
				for(k = j*term+1;k < j*term + 1 + term; k++){
					if (l == words[i][j]) fprintf(psddTrain,"1,");
					else fprintf(psddTrain,"0,"); 
					l++;
				}
			}
			for (j = len*term+1;j < term*len + nonTerm*len*(len+1)/2 + 1;j++){
				if (j != term*len + nonTerm*len*(len+1)/2) fprintf(psddTrain,"-1,");
				else fprintf(psddTrain,"-1");
			}
			fprintf(psddTrain,"\n");
		}

		
		createSet(len,maxRuleCount,nonTerm,rules,setSize,words,term,start);

		//1 indexed for sdd literals
		int sddLits[setSize][term*len + nonTerm*len*(len+1)/2 + 1];


		char pcfgTestBuf[32];
		snprintf(pcfgTestBuf, sizeof(char) * 32, "./pcfgTesting%i.txt", runNum);
		char psddTestBuf[32];
		snprintf(psddTestBuf, sizeof(char) * 32, "./psddTesting%i.txt", runNum);
		FILE *pcfgTest;
		pcfgTest = fopen(pcfgTestBuf, "w");
		FILE *psddTest;
		psddTest = fopen(psddTestBuf,"w");

		/*
		for (i = 0; i < setSize; i++){
			for (j = 0; j < len; j++){
				printf("%d",words[i][j]);
				if (j != len-1) printf(",");
			}
			printf("\n");
		}


		for (i = 0; i < setSize; i++){
			for (j = 0; j < len; j++){
				fprintf(pcfgTest,"%d",words[i][j]);
				if (j != len-1) fprintf(pcfgTest,",");
			}
			fprintf(pcfgTest,"\n");
		}
		*/
		

		for(i = 0; i < setSize; i++){
			for(j = 0; j < len; j++){
				int theWord = words[i][j];
				int done = 0;
				switch (theWord)
				{
					case 0:
						fprintf(pcfgTest,"a");
						break;
					case 1:
						fprintf(pcfgTest,"b");
						break;
					case 2:
						fprintf(pcfgTest,"x");
						break;
					case 3:
						fprintf(pcfgTest,"y");
						break;		
				}
				if (j != len-1) fprintf(pcfgTest," ");
			}
			fprintf(pcfgTest,"\n");
		}

		for (i = 0; i < setSize; i++){
			for (j = 0; j < len; j++){
				l = 0;
				for(k = j*term+1;k < j*term + 1 + term; k++){
					if (l == words[i][j]) fprintf(psddTest,"1,");
					else fprintf(psddTest,"0,"); 
					l++;
				}
			}
			for (j = len*term+1;j < term*len + nonTerm*len*(len+1)/2 + 1;j++){
				if (j != term*len + nonTerm*len*(len+1)/2) fprintf(psddTest,"-1,");
				else fprintf(psddTest,"-1");
			}
			fprintf(psddTest,"\n");
		}
		fclose(psddTest);
		fclose(pcfgTest);
		fclose(psddTrain);
		fclose(pcfgTrain);
	}
	
	/*
	//for createSet
	for(i = 0; i < setSize; i++){
		for(j = 0; j < len; j++){
			int theWord = words[i][j];
			int done = 0;
			switch (theWord)
			{
				case 0:
					printf("a");
					break;
				case 1:
					printf("b");
					break;
				case 2:
					printf("x");
					break;
				case 3:
					printf("y");
					break;		
			}
			if (j != len-1) printf(" ");
		}
		printf("\n");
	}
	
	//for createRandom
	/*
	for(i = 0; i < setSize; i++){
		for(j = 0; j < len; j++){
			int theWord = words[i][j];
			int done = 0;
			switch (theWord)
			{
				case 0:
					printf("a");
					break;
				case 1:
					printf("b");
					break;
				case 2:
					printf("x");
					break;
				case 3:
					printf("y");
					break;
				case -1:
					done = 1;
					break;		
			}
			if (done){
				break;
			}
			if (j != len-1) printf(" ");
		}
		printf("\n");
	}
	*/
	
	
	
	//printf("%d\n",words[0][2]);
}