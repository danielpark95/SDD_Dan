/****************************************************************************************
 * The Sentential Decision Diagram Package
 * sdd version 1.1.1, January 31, 2014
 * http://reasoning.cs.ucla.edu/sdd
 ****************************************************************************************/

#include "sddapi.h"
#include "compiler.h"
#include "parameters.h"

/****************************************************************************************
 * this file contains the dynamic vtree search algorithm
 ****************************************************************************************/
 
//swapping the values of two variables
//
//T: type of vars
//V1: first var
//V2: second var
#define SWAP(T,V1,V2) {	T V = V1; V1 = V2; V2 = V; }

// forward references from search_state.c
int is_virtual_leaf_vtree(Vtree* vtree);
Vtree* update_vtree_change(Vtree* vtree, SddManager* manager);
Vtree* update_vtree_change_p(Vtree* vtree, SddManager* manager);

/****************************************************************************************
 * local search algorithm for finding good vtrees dynamically
 *
 *
 * in the following, if a vtree node is not leaf, we will call it internal
 *
 * for each node in the vtree, call it root, we examine up to 24 vtree variations and 
 * adopt the one vtree leading to the smallest sdd size
 *
 * the 24 variations can be divided into two groups, called L and R
 *
 * group L:
 * --contains 12 variations and is defined only if root r has an internal left child c
 * --it corresponds to all vtrees with three leaves corresponding to: left(c), right(c), right(r)
 *
 * group R:
 * --contains 12 variations and is defined only if root r has an internal right child c 
 * --it corresponds to all vtrees having three leaves corresponding to: left(r), left(c), right(c)
 *
 * for a given root, we then examine:
 *
 *  --24 variations, L+R, if both its left and right children of root are internal
 *  --12 variations, L,   if left child is internal, but right child is leaf
 *  --12 variations, R,   if left child is leaf, but right child is internal
 *  --0 variations,       if root is leaf, or if both its children are leaves
 *
 * as a result, no variations are examined if root has < 3 leaves below it
 *
 * each iteration of the search algorithm will navigate the above variations for every 
 * node in the vtree, visiting vtree nodes bottom up
 *
 * if an iteration leads to reducing the sdd size by more than a certain threshold,
 * another iteration is applied to the vtree
 *
 * one can navigate through variations using left rotate, right rotate and swap
 *
 * the navigation algorithm maintains two variables:
 *  --root
 *  --child
 * and uses the following three moves:
 * 
 * 'l': left rotate(child)
 * 'r': right rotate(root)
 * 's': sdd_vtree_swap(child)
 *
 * if, initially, child is pointing to the left child of root, then the move sequence
 * stored in array moves_ll[] will navigate through all variations in group L (i.e., last
 * move takes us back to initial vtree)
 *
 * if, initially, child is pointing to the right child of root, then the move sequence
 * stored in moves_rl[] will navigate through all variations in group R (i.e., last move
 * takes us back to initial vtree)
 *
 * the algorithm updates the values of root and child after every move                  
 *
 *
 * moves_ll[12]: 
 * --move sequence for navigating variations in group L
 * --the vtree sequence navigated by this sequence corresponds to all vtrees with 3 leaves,
 *   starting with a LEFT-linear vtree
 *
 * moves_rl[12]: 
 * --move sequence for navigating variations in group R
 * --the vtree sequence navigated by this sequence corresponds to all vtrees with 3 leaves,
 *   starting with a RIGHT-linear vtree
 *
 * the algorithm can take advantage of time or size limits when applying its moves:
 *
 * --time limit: a move will be aborted if its time exceeds a given limit
 * --size limit: a move will be aborted it it increases sdd size by a given factor
 *
 * under time or size limits, some variations in a group may not be visited even
 * if the group is defined
 *
 * see headers/dynamic.c for setting time and size limits
 *
 ***************************************************************************************/

//note: assuming the moves are numbered from 0 to 11, 
//the six distinct total orders are generated after moves 1, 3, 5, 7, 9, 11
char moves_ll[12] = {'r','s','l','s',   'r','s','l','s',   'r','s','l','s'}; 
char moves_rl[12] = {'l','s','r','s',   'l','s','r','s',   'l','s','r','s'};

/****************************************************************************************
 * making or reversing single moves
 ****************************************************************************************/
 
//try move with time or size limits
//move applied to linear vtree whose root is root and whose internal node is child
//if move succeeds, return 1, otherwise return 0
int try_move(char move, Vtree** root, Vtree** child, SddManager* manager) {

  if(move=='l') {
    assert(*child==sdd_vtree_right(*root));
    if(sdd_vtree_rotate_left(*child,manager,TIME_LIMIT_LR,SIZE_LIMIT_LR)) {
      //left rotation succeeded
      SWAP(Vtree*,*root,*child); //root/child flip positions
      return 1;
    }
  }
  else if(move=='r') {
    assert(*child==sdd_vtree_left(*root));
    if(sdd_vtree_rotate_right(*root,manager,TIME_LIMIT_RR,SIZE_LIMIT_RR,CARTESIAN_PRODUCT_LIMIT)) {
      //right rotation succeeded
      SWAP(Vtree*,*root,*child); //root/child flip positions
      return 1;
    }
  }
  else { //move=='s'
    assert(*root==sdd_vtree_parent(*child));
    if(sdd_vtree_swap(*child,manager,TIME_LIMIT_SW,SIZE_LIMIT_SW,CARTESIAN_PRODUCT_LIMIT)) {
      //swap succeeded, root/child stay the same
      return 1;
    } 
  } 
  
  return 0; //move failed
}

//make move without time or size limits
//move applied to linear vtree whose root is root and whose internal node is child
//move will always succeed
int make_move(char move, Vtree** root, Vtree** child, SddManager* manager) {
  
  if(move=='l') {
    assert(*child==sdd_vtree_right(*root));
    sdd_vtree_rotate_left(*child,manager,0,0);
    SWAP(Vtree*,*root,*child); //root/child flip positions
  }
  else if(move=='r') {
    assert(*child==sdd_vtree_left(*root));
    sdd_vtree_rotate_right(*root,manager,0,0,0);
    SWAP(Vtree*,*root,*child); //root/child flip positions
  }
  else { //move=='s'
    assert(*root==sdd_vtree_parent(*child));
    sdd_vtree_swap(*child,manager,0,0,0); //root/child stay the same
  }
  
  return 1;
}

//reverse move without time or size limits
//reversed move applied to linear vtree whose root is root and whose internal node is child
//reversal will always succeed
void reverse_move(char move, Vtree** root, Vtree** child, SddManager* manager) {
  assert(*root==sdd_vtree_parent(*child));
  
  if(move=='l') make_move('r',root,child,manager);
  else if(move=='r') make_move('l',root,child,manager);
  else make_move('s',root,child,manager); //move=='s'
}

/****************************************************************************************
 * making or reversing move sequences
 ****************************************************************************************/
 
//make moves from 0 to loc without time or size limits
//the procedure implements some shortcuts which utilize swapping the root
void make_moves(int loc, char* moves, Vtree** root, Vtree** child, SddManager* manager) {
  assert(*root==sdd_vtree_parent(*child));
  
  int i = 0;
  while(i <= loc) {
    if(i==0 && (loc == 4 || loc == 5)) {
      sdd_vtree_swap(*root,manager,0,0,0); //shortcut for moves 0..4
      i=5;
    } 
    else if(i==1 && loc >= 7) {
      sdd_vtree_swap(*root,manager,0,0,0); //shortcut for moves 1..7
      i=8;
    } 
    else if(i==2 && loc >= 6) {
      sdd_vtree_swap(*root,manager,0,0,0); //shortcut for moves 2..6
      i=7;
    } 
    else if(i==3 && loc >= 10) {
      sdd_vtree_swap(*root,manager,0,0,0); //shortcut for moves 3..9
      i=10;
    } 
    else make_move(moves[i++],root,child,manager);
  }

}

//reverse move at end, then end-1, .., then 0, without time or size limits
//the procedure implements some shortcuts which utilize swapping the root
void reverse_moves(int end, char* moves, Vtree** root, Vtree** child, SddManager* manager) {
  assert(*root==sdd_vtree_parent(*child));
  
  while(end >= 0) {
   if(end==10) {
      sdd_vtree_swap(*root,manager,0,0,0); //shortcut for reversing moves 10..6
      end=5;
    }
    else if(end==9) {
      sdd_vtree_swap(*root,manager,0,0,0); //shortcut for reversing moves 9..3
      end=2;
    }
    else if(end==7) {
      sdd_vtree_swap(*root,manager,0,0,0); //shortcut for reversing moves 7..1
      end=0;
    }
    else if(end==6) {
      sdd_vtree_swap(*root,manager,0,0,0); //shortcut for reversing moves 6..2
      end=1;
    }
    else if(end==4) {
      sdd_vtree_swap(*root,manager,0,0,0); //shortcut for reversing moves 4..0
      return;
    } 
    else reverse_move(moves[end--],root,child,manager);
  }
  
}
 
/****************************************************************************************
 * navigating vtrees in group L or group R
 ****************************************************************************************/

static inline
SddLiteral balance(Vtree* vtree) {
  SddLiteral left_count = sdd_vtree_var_count(sdd_vtree_left(vtree));
  SddLiteral right_count = sdd_vtree_var_count(sdd_vtree_right(vtree));
  return abs(left_count - right_count);
}

//returns index of last move in sequence leading to best vtree
//if -1 is returned, then original vtree is best
int find_best_of_12_local_vtrees(char* moves, SddSize* best_size, SddSize* best_count, SddLiteral* best_balance, Vtree** root, Vtree** child, SddManager* manager) {
  assert(*root==sdd_vtree_parent(*child));
  int best = -1; //initial vtree
  
  //new initial size for enforcing size limits
//  sdd_manager_update_size_limit_context(manager);
  
  for(int i=0; i<12; i++) { //12 vtrees (move i=11 leads back to initial vtree)
    //i==11 takes us back to the initial vtree so no point in allowing this move to fail
    if((i<11 && try_move(moves[i],root,child,manager)) || (i==11 && make_move(moves[i],root,child,manager))) {
      
      //move i succeeded
      SddSize cur_size       = sdd_manager_live_size(manager);
      SddSize cur_count      = sdd_manager_live_count(manager);
      SddLiteral cur_balance = balance(*root);

      //see if we improved on last best
      if(cur_size <*best_size || (cur_size==*best_size && (cur_balance<*best_balance || cur_count<*best_count))) {
        //new initial size for enforcing size limits
        sdd_manager_update_size_limit_context(manager);
        //improvement
        *best_size    = cur_size;
        *best_count   = cur_count;
        *best_balance = cur_balance;
        best = i; //keep track of move sequence leading to the sdd with this best size
      }
    }
    else {
      //move i failed due to time or size limit, so current vtree did not change
      //reverse moves 0..i-1 that have lead to current vtree
      reverse_moves(i-1,moves,root,child,manager);
      //since we failed to navigate all 12 vtrees due to time or size limits, just
      //return the move sequence leading to the best vtree found under the given limits
      return best; 
    }
  }
  
  //return the move sequence leading to the best vtree among all 12 vtrees navigated
  return best;
}

/****************************************************************************************
 * navigating variations in groups L and R
 ****************************************************************************************/

//returns root of best vtree found
Vtree* best_of_24_local_vtrees(Vtree* vtree, SddManager* manager) {

  int best_rl = -1; //initial right-linear vtree
  int best_ll = -1; //initial left-linear vtree
  SddSize best_size       = sdd_manager_live_size(manager);
  SddSize best_count      = sdd_manager_live_count(manager);
  SddLiteral best_balance = balance(vtree);
  Vtree* root = vtree;
  
  if(!is_virtual_leaf_vtree(sdd_vtree_right(root))) { //group R is defined
    //check 12 vtrees, starting with a right-linear vtree
    Vtree* child = sdd_vtree_right(root);
    best_rl = find_best_of_12_local_vtrees(moves_rl,&best_size,&best_count,&best_balance,&root,&child,manager);
    //should be back where we started
    assert(child==sdd_vtree_right(root));
  }
 
  if(!is_virtual_leaf_vtree(sdd_vtree_left(root))) { //group L is defined
    //check 12 vtrees, starting with a left-linear vtree
    Vtree* child = sdd_vtree_left(root);
    best_ll = find_best_of_12_local_vtrees(moves_ll,&best_size,&best_count,&best_balance,&root,&child,manager);
    //should be back where we started
    assert(child==sdd_vtree_left(root));
  }
  
  //the following order of test cases is CRITICAL
  if(best_ll != -1) {
    //the best vtree found is part of group L
    Vtree* child = sdd_vtree_left(root);
    //navigate to the best vtree found
    make_moves(best_ll,moves_ll,&root,&child,manager);
    assert(root==sdd_vtree_parent(child)); //child may be left or right now
  }
  else if(best_rl != -1) {
    //the best vtree found is part of group R
    Vtree* child = sdd_vtree_right(root);
    //navigate to the best vtree found
    make_moves(best_rl,moves_rl,&root,&child,manager);
    assert(root==sdd_vtree_parent(child)); //child may be left or right now
  }

  assert(best_size==sdd_manager_live_size(manager));
  assert(best_count==sdd_manager_live_count(manager));
  
  return root;
  //note: due to time or size limits, may not have visited all 24 vtrees
}

/****************************************************************************************
 * applying an iteration of the search algorithm
 ****************************************************************************************/
 
//returns root of best vtree found
Vtree* local_search_pass(Vtree* vtree, SddManager* manager) {

  if(is_virtual_leaf_vtree(vtree)) return vtree;

  local_search_pass(sdd_vtree_left(vtree),manager);
  local_search_pass(sdd_vtree_right(vtree),manager);
      
  return best_of_24_local_vtrees(vtree,manager);

}

/****************************************************************************************
 * dynamic vtree search 
 ****************************************************************************************/

//returns new root
Vtree* vtree_search(Vtree* vtree, SddManager* manager) {
  if(sdd_vtree_is_leaf(vtree)) return vtree;

  sdd_vtree_garbage_collect(vtree,manager); //local garbage collection
  
  Vtree* subtree = update_vtree_change(vtree,manager); //identify a changed subtree
  if(subtree==NULL) return vtree; //no change found in vtree

  sdd_manager_set_size_limit_context(subtree,manager); //context for size limits
  //using subtree instead of vtree above leads to less search
  
  //set up
  Vtree** root_location      = sdd_vtree_location(vtree,manager); //remember root location
  SddSize init_size          = sdd_vtree_live_size(vtree); //size of sdd for vtree 
  //using vtree instead of subtree above leads to less search
  SddSize prev_size          = init_size;
  SddSize cur_size           = init_size; //to avoid compiler warning
  SddSize outside_size       = sdd_manager_live_size(manager)-init_size; //constant thru iterations 
  float reduction            = 100;
  
  //iterate
  do {
    subtree   = local_search_pass(subtree,manager); //possibly new root
    cur_size  = sdd_manager_live_size(manager)-outside_size; //new size
    reduction = (100.0*(prev_size-cur_size))/init_size; //pertentage of size reduction
    prev_size = cur_size;
    subtree   = update_vtree_change_p(subtree,manager); //identify a changed subtree
    //also updates state of subtree's parent
  }
  while(subtree!=NULL && reduction >= DYNAMIC_VTREE_CONVERGENCE_THRESHOLD);

  return *root_location; //root may have changed due to rotations
}


/****************************************************************************************
 * end
 ****************************************************************************************/
