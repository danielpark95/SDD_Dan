/****************************************************************************************
 * The Sentential Decision Diagram Package
 * sdd version 1.1.1, January 31, 2014
 * http://reasoning.cs.ucla.edu/sdd
 ****************************************************************************************/

#include "sddapi.h"
#include "compiler.h"
#include "parameters.h"

/****************************************************************************************
 * this file contains the fnf-to-sdd compiler, with AUTO gc and sdd-minimize
 *
 * NOTE: this file is currently set to use the vtree search algorithm distributed
 * with the SDD package. this is meant to allow users to modify this search algorithm,
 * with the hope that they will improve on it.
 *
 * by commenting in/out TWO lines of code below, this can be changed to use the vtree 
 * search algorithm built into the SDD library (look for SWITCH-TO-LIBRARY-SEARCH).
 *
 * the two algorithms are identical though, so the results should match. 
 ****************************************************************************************/

// local declarations
static SddNode* apply_vtree_auto(Vtree* vtree, BoolOp op, SddManager* manager);
SddNode* apply_litset_auto(LitSet* litset, SddManager* manager);
  

/****************************************************************************************
 * compiles a cnf or dnf into an sdd
 ****************************************************************************************/

//fnf is either a cnf or a dnf
SddNode* fnf_to_sdd_auto(Fnf* fnf, SddManager* manager) {
  sdd_manager_auto_gc_and_minimize_on(manager);
  //to SWITCH-TO-LIBRARY-SEARCH, comment in the next line, comment out the one after
  sdd_manager_set_minimize_function(vtree_search,manager); //user-defined search algorithm
//  sdd_manager_set_minimize_function(sdd_vtree_minimize,manager); //library's search algorithm
  
  distribute_fnf_over_vtree(fnf,manager);
  SddNode* node = apply_vtree_auto(sdd_manager_vtree(manager),fnf->op,manager);
  free_vtree_data(sdd_manager_vtree(manager)); //root may have changed
  
  return node;
}

//each vtree node is associated with a set of litsets (clauses or terms)
//the totality of these litsets represent an fnf (cnf or dnf)
//returns an sdd which is equivalent to the cnf/dnf associated with vtree
SddNode* apply_vtree_auto(Vtree* vtree, BoolOp op, SddManager* manager) {
  //get litsets associated with vtree node
  //do this first as vtree root may be changed by dynamic vtree search
  LitSet** litsets     = DATA(vtree,litsets);
  SddSize litset_count = DATA(vtree,litset_count);  
  
  sort_litsets_by_lca(litsets,litset_count,manager);

  SddNode* node;
  if(sdd_vtree_is_leaf(vtree)) node = ONE(manager,op);
  else {
    SddNode* l_node = apply_vtree_auto(sdd_vtree_left(vtree),op,manager);
    sdd_ref(l_node,manager);
    SddNode* r_node = apply_vtree_auto(sdd_vtree_right(vtree),op,manager);
    sdd_deref(l_node,manager);
    node = sdd_apply(l_node,r_node,op,manager);
  }
  
  while(litset_count--) { //compile and integrate litset
    sdd_ref(node,manager);
    SddNode* litset = apply_litset_auto(*litsets++,manager); //may gc node
    sdd_deref(node,manager);
   
    node = sdd_apply(litset,node,op,manager);
        
    //recompute lcas of remaining clauses and sort again
    sort_litsets_by_lca(litsets,litset_count,manager);
  }
  
  return node;
}

//converts a clause/term into an equivalent sdd
SddNode* apply_litset_auto(LitSet* litset, SddManager* manager) {

  BoolOp op            = litset->op; //conjoin (term) or disjoin (clause)
  SddLiteral* literals = litset->literals;
  SddNode* node        = ONE(manager,op); //will not be gc'd
  
  for(SddLiteral i=0; i<litset->literal_count; i++) {
    SddNode* literal = sdd_manager_literal(literals[i],manager);
    node             = sdd_apply(node,literal,op,manager);
  }
  
  return node;
}

/****************************************************************************************
 * end
 ****************************************************************************************/
