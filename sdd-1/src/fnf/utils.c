/****************************************************************************************
 * The Sentential Decision Diagram Package
 * sdd version 1.1.1, January 31, 2014
 * http://reasoning.cs.ucla.edu/sdd
 ****************************************************************************************/

#include "sddapi.h"
#include "compiler.h"

/****************************************************************************************
 * a key concept in this file is the "litset lca":
 *  --a litset is a set of literals, representing a clause or a term
 *  --the "litset lca" is the lowest vtree node that contains all variables of a litset
 *
 * the litset lca is unique and is stored in the vtree field of a litset
 *
 * this file contains two main functions that:
 * 1. distribute the litsets of an fnf over vtree nodes (assigns each litset to its lca)
 * 2. sort an array of litsets according to their lca (details below)
 ****************************************************************************************/

/****************************************************************************************
 * the data field of a vtree is used to store litsets (clauses or terms)
 *
 * initializing and freeing data fields
 ****************************************************************************************/

void initialize_vtree_data(Vtree* vtree) {
  void* data;
  NEW_DATA(data);
  sdd_vtree_set_data(data,vtree);
  DATA(vtree,litset_count) = 0;
  DATA(vtree,litsets) = NULL;
  if(!sdd_vtree_is_leaf(vtree)) {
    initialize_vtree_data(sdd_vtree_left(vtree));
    initialize_vtree_data(sdd_vtree_right(vtree));
  }
}

//free the fnf associated with a vtree
void free_vtree_data(Vtree* vtree) {
  if(!sdd_vtree_is_leaf(vtree)) {
    free_vtree_data(sdd_vtree_left(vtree));
    free_vtree_data(sdd_vtree_right(vtree));
  }
  FREE_DATA(vtree);
}

/****************************************************************************************
 * given an fnf that consists of a number of litsets (i.e., clauses or terms), 
 * distribute these litsets over the vtree nodes corresponding to their lcas
 ****************************************************************************************/

//distribute fnf litsets over vtree nodes
//the litsets associated with vtree nodes must form a partition of the fnf litsets
void distribute_fnf_over_vtree(Fnf* fnf, SddManager* manager) {
  initialize_vtree_data(sdd_manager_vtree(manager));
  
  for(SddSize i=0; i<fnf->litset_count; i++) {
    LitSet* litset = fnf->litsets +i;
    Vtree* lca = sdd_manager_lca_of_literals(litset->literal_count,litset->literals,manager);
    
    //increase size of array holding litsets at lca
    SddLiteral count  = DATA(lca,litset_count);
    DATA(lca,litsets) = realloc(DATA(lca,litsets),(count+1)*sizeof(LitSet*));
    
    //add litset to its lca
    DATA(lca,litsets)[count] = litset;
    DATA(lca,litset_count)   = 1+count;
  }
  
}

/****************************************************************************************
 * sort litsets according to their lcas
 ****************************************************************************************/

int litset_cmp_lca(const void* litset1_loc, const void* litset2_loc) {

  LitSet* litset1 = *(LitSet**)litset1_loc;
  LitSet* litset2 = *(LitSet**)litset2_loc;

  Vtree* vtree1 = litset1->vtree;
  Vtree* vtree2 = litset2->vtree;
  SddLiteral p1 = sdd_vtree_position(vtree1);
  SddLiteral p2 = sdd_vtree_position(vtree2);
  
  if(vtree1!=vtree2 && (sdd_vtree_is_sub(vtree2,vtree1) || (!sdd_vtree_is_sub(vtree1,vtree2) && (p1 > p2)))) return 1;
  else if(vtree1!=vtree2 && (sdd_vtree_is_sub(vtree1,vtree2) || (!sdd_vtree_is_sub(vtree2,vtree1) && (p1 < p2)))) return -1;
  else {
	
  SddLiteral l1 = litset1->literal_count;
  SddLiteral l2 = litset2->literal_count;
  
  if(l1 > l2) return 1;
  else if(l1 < l2) return -1;
  else { 
    //so the litset order is unique
  	//without this, final litset order may depend on system
    SddSize id1 = litset1->id;
    SddSize id2 = litset2->id;
    if(id1 > id2) return 1;
    else if(id1 < id2) return -1;
    else return 0;
  }
  }
}

//first: incomparable lcas are left to right, comparabale lcas are top to down
//then: shorter to larger litsets
//last: by id to obtain unique order
void sort_litsets_by_lca(LitSet** litsets, SddSize size, SddManager* manager) {
  //compute lcas of litsets
  for(SddLiteral i=0; i<size; i++) {
    LitSet* litset = litsets[i];
    litset->vtree  = sdd_manager_lca_of_literals(litset->literal_count,litset->literals,manager);
  }
  //sort
  qsort((LitSet**)litsets,size,sizeof(LitSet*),litset_cmp_lca);
}

/****************************************************************************************
 * end
 ****************************************************************************************/
