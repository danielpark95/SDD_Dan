#include <stdio.h>
#include <stdlib.h>
#include "sddapi.h"


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

SddNode* oneInTen(SddManager* m){
	SddNode* delta = sdd_manager_false(m);
	SddNode* alpha;
	int i,j;
	for(i = 1; i < 11; i++) {
		alpha = sdd_manager_true(m);
		for(j = 1; j < 11; j++){
			if (j == i){
				alpha = sdd_conjoin(sdd_manager_literal(j,m),alpha,m);
			}
			else{
				alpha = sdd_conjoin(sdd_manager_literal(-j,m),alpha,m);
			}
		}
		delta = sdd_disjoin(alpha,delta,m);
	}
	return delta;
}

SddNode* rnFor(SddManager* m, int r, int n){
	SddNode* dy[r+1][n+1];
	int i,j;
	dy[0][0] = sdd_manager_true(m);
	for(j=1;j<=n;j++){
		dy[0][j] = sdd_conjoin(sdd_manager_literal(-j,m),dy[0][j-1],m);
	}
	for(i=1;i<=r;i++){
		dy[i][i-1] = sdd_manager_false(m);
	}
	for (i=1;i<=r;i++){
		for(j=i;j<=n;j++){
			dy[i][j] = sdd_disjoin(sdd_conjoin(sdd_manager_literal(-j,m),dy[i][j-1],m),sdd_conjoin(sdd_manager_literal(j,m),dy[i-1][j-1],m),m);
		}
	}
	return dy[r][n];
}

SddNode* rnGC(SddManager* m, int r, int n){
	SddNode* dy[r+1][n+1];
	int i,j;
	dy[0][0] = sdd_manager_true(m);
	for(j=1;j<=n;j++){
		dy[0][j] = sdd_conjoin(sdd_manager_literal(-j,m),dy[0][j-1],m);
		//printEverything(m);
		sdd_ref(dy[0][j],m);
		//printf("after\n");
		//printEverything(m);
	}
	printf("hi\n");
	for(i=1;i<=r;i++){
		dy[i][i-1] = sdd_manager_false(m);
	}
	for (i=1;i<=r;i++){
		for(j=i;j<=n;j++){
			SddNode* a = sdd_conjoin(sdd_manager_literal(-j,m),dy[i][j-1],m);
			sdd_ref(a,m);
			SddNode* b = sdd_conjoin(sdd_manager_literal(j,m),dy[i-1][j-1],m);
			sdd_ref(b,m);
			dy[i][j] = sdd_disjoin(a,b,m);//ref a & b, then deref after creating c
			sdd_ref(dy[i][j],m); sdd_deref(a,m); sdd_deref(b,m);
		}
	}
	
	for(j=1;j<=n;j++){
		
		//printEverything(m);
		sdd_deref(dy[0][j],m);
		//printf("after\n");
		//printEverything(m);
		
	}
	for (i=1;i<=r;i++){
		for(j=i;j<=n;j++){
			sdd_deref(dy[i][j],m);
		}
	}
	return dy[r][n];//deref before returning
}

/*
SddNode* parityOdd(SddManager* m, int n){
	SddNode* dy[]
}
*/


int main(int argc, char** argv) {
	// initialize manager
	SddLiteral var_count = 10; // initial number of variables
	int auto_gc_and_minimize = 1; // disable (0) or enable (1) auto-gc & auto-min
	//SddManager* m = sdd_manager_create(var_count,auto_gc_and_minimize);
	
	
	Vtree* vtree = sdd_vtree_new(var_count, "right");
	SddManager* m = sdd_manager_new(vtree);
	
	
	
	// CONSTRUCT, MANIPULATE AND QUERY SDDS
	
	SddLiteral A=1,B=2,C=3,D=4,E=5,F=6,G=7,H=8,I=9,J=10;
	
	int r = 3;
	int n = 10;
	
	//SddNode* delta = rnFor(m,r,n);//ref after setting
	
	SddNode* delta = rnGC(m,r,n);//ref after setting
	sdd_ref(delta,m);
	
	/*
	delta = sdd_condition(A,delta,m);
	delta = sdd_condition(-B,delta,m);
	delta = sdd_condition(C,delta,m);
	delta = sdd_condition(-D,delta,m);
	delta = sdd_condition(-E,delta,m);
	delta = sdd_condition(F,delta,m);
	delta = sdd_condition(G,delta,m);
	delta = sdd_condition(H,delta,m);
	delta = sdd_condition(-I,delta,m);
	delta = sdd_condition(-J,delta,m);
	
	
	int is_abnormal = sdd_node_is_false(delta);
	
	if (is_abnormal) {
		printf("abnormal");
		printf("\n");
	}
	else {
		printf("normal");
		printf("\n");
	}
	
	//*/
	
	SddModelCount count = sdd_model_count(delta,m);
	int i = count;
	printf("model count: %d\n",i);
	
	
	printEverything(m);
	
	sdd_save_as_dot("sdd1.dot",delta);
	
	sdd_deref(delta,m);
	printEverything(m);
	// free manager
	sdd_manager_free(m);
	return 0;
}