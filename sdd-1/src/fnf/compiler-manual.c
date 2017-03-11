/****************************************************************************************
 * The Sentential Decision Diagram Package
 * sdd version 1.1.1, January 31, 2014
 * http://reasoning.cs.ucla.edu/sdd
 ****************************************************************************************/

#include "sddapi.h"
#include "compiler.h"
#include "parameters.h"

/****************************************************************************************
 * this file contains the fnf-to-sdd compiler, with MANUAL gc and sdd-minimize
 *
 * NOTE: this file is currently set to use the vtree search algorithm distributed
 * with the SDD package. this is meant to allow users to modify this search algorithm,
 * with the hope that they will improve on it.
 *
 * by commenting in/out FOUR lines of code below, this can be changed to use the vtree 
 * search algorithm built into the SDD library (look for SWITCH-TO-LIBRARY-SEARCH).
 *
 * the two algorithms are identical though, so the results should match. 
 ****************************************************************************************/

// local declarations
static SddNode* apply_vtree_manual(Vtree* vtree, BoolOp op, SddManager* manager);
static SddNode* apply_litsets_manual(SddNode* base, BoolOp op, Vtree* vtree, SddManager* manager);

/****************************************************************************************
 * compiles a cnf or dnf into an sdd
 ****************************************************************************************/

//fnf is either a cnf or a dnf
SddNode* fnf_to_sdd_manual(Fnf* fnf, SddManager* manager) {
  
  distribute_fnf_over_vtree(fnf,manager);
  SddNode* node = apply_vtree_manual(sdd_manager_vtree(manager),fnf->op,manager);
  free_vtree_data(sdd_manager_vtree(manager)); //root may have changed
  
  return node;
}

//each vtree node is associated with a set of litsets (clauses or terms)
//the totality of these litsets represent an fnf (cnf or dnf)
//returns an sdd which is equivalent to the cnf/dnf associated with vtree
SddNode* apply_vtree_manual(Vtree* vtree, BoolOp op, SddManager* manager) {
  SddNode* base;

  if(sdd_vtree_is_leaf(vtree)) base = ONE(manager,op);
  else {
    SddNode* l_node = apply_vtree_manual(sdd_vtree_left(vtree),op,manager);
    SddNode* r_node = apply_vtree_manual(sdd_vtree_right(vtree),op,manager);
    base            = sdd_apply_in_vtree(l_node,r_node,op,vtree,manager);
  }

  SddSize litset_count = DATA(vtree,litset_count);
  if(litset_count==0) return base; //no clauses/terms stored at vtree node
  
  //apply litsets may change root of vtree due to vtree search
  Vtree** vtree_loc = sdd_vtree_location(vtree,manager);
  SddNode* node     = apply_litsets_manual(base,op,vtree,manager);
  vtree             = *vtree_loc; //root may have changed

  SddManagerOptions* options = sdd_manager_options(manager);
  if(options->vtree_search_mode==2) {
    sdd_ref(node,manager);
    //to SWITCH-TO-LIBRARY-SEARCH, comment in the next line, comment out the one after
    vtree_search(vtree,manager);
//    sdd_vtree_minimize(vtree,manager); //library's version of vtree search algorithm
    sdd_deref(node,manager);
  }
  
  return node;
}

//converts a clause/term into an equivalent sdd
//all variables of litset must appear in vtree
SddNode* apply_litset_manual(LitSet* litset, Vtree* vtree, SddManager* manager) {
  BoolOp op            = litset->op; //conjoin (term) or disjoin (clause)
  SddLiteral* literals = litset->literals;
  SddNode* node        = ONE(manager,op);
  
  for(SddLiteral i=0; i<litset->literal_count; i++) {
    SddNode* literal = sdd_manager_literal(literals[i],manager);
    node             = sdd_apply_in_vtree(node,literal,op,vtree,manager);
  }
  
  return node;
}

//convert the clauses/terms associated with vtree into sdds and combine them with base using op
SddNode* apply_litsets_manual(SddNode* base, BoolOp op, Vtree* vtree, SddManager* manager) {

  //get litsets associated with vtree node
  LitSet** litsets     = DATA(vtree,litsets);
  SddSize litset_count = DATA(vtree,litset_count);
  
  assert(litset_count!=0);
  sort_litsets_by_lca(litsets,litset_count,manager);
  
  SddNode* result = base;
  sdd_ref(result,manager);
    
  //must be done after referencing
  SddSize prev_size = sdd_vtree_live_size(vtree);

  SddManagerOptions* options = sdd_manager_options(manager);
  while(litset_count--) {
    //compile and integrate litset
    SddNode* node = apply_litset_manual(*litsets++,vtree,manager);
    sdd_deref(result,manager);
    result        = sdd_apply_in_vtree(node,result,op,vtree,manager);
    sdd_ref(result,manager);

    if(options->vtree_search_mode==2) {
      SddSize cur_size  = sdd_vtree_live_size(vtree); //after integrating last litset
      if(cur_size  > prev_size*options->vtree_search_threshold) {
        //to SWITCH-TO-LIBRARY-SEARCH, comment in the next line, comment out the one after
        vtree = vtree_search(vtree,manager); // root of vtree may have changed
//        vtree = sdd_vtree_minimize(vtree,manager); //library's version of vtree search algorithm
        prev_size  = sdd_vtree_live_size(vtree); //since last call to dynamic vtree
        //recompute lcas of remaining clauses and sort again
        sort_litsets_by_lca(litsets,litset_count,manager);
      }
    }
      
    sdd_vtree_garbage_collect_if(options->gc_threshold,vtree,manager);
  }
 
  sdd_deref(result,manager);

  return result;
}

/****************************************************************************************
 * end
 ****************************************************************************************/
