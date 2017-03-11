/****************************************************************************************
 * The Sentential Decision Diagram Package
 * sdd version 1.1.1, January 31, 2014
 * http://reasoning.cs.ucla.edu/sdd
 ****************************************************************************************/

#include <time.h>
#include "sddapi.h"
#include "compiler.h"

// forward references
SddCompilerOptions sdd_getopt(int argc, char **argv);
char* ppc(SddSize n); // pretty print

SddNode* fnf_to_sdd_auto(Fnf* fnf, SddManager* manager);
SddNode* fnf_to_sdd_manual(Fnf* fnf, SddManager* manager);
void free_fnf(Fnf* fnf);

void* initialize_manager_search_state(SddManager* manager);
void free_manager_search_state(SddManager* manager);
Vtree* vtree_search(Vtree* vtree, SddManager* manager);

/****************************************************************************************
 * start
 ****************************************************************************************/
 
SddManager* cm;

int main(int argc, char** argv) {

  //get options from command line (and defaults)
  SddCompilerOptions options = sdd_getopt(argc,argv);

  Fnf* fnf;
  Vtree* vtree;
  SddNode* node;
  SddManager* manager;
  clock_t c1, c2;

  if(options.cnf_filename!=NULL) {
    printf("\nreading cnf...");
    fnf = sdd_cnf_read(options.cnf_filename);
    printf("vars=%"PRIlitS" clauses=%"PRIsS"",fnf->var_count,fnf->litset_count);
  }
  else { //options.dnf_filename!=NULL
    printf("\nreading dnf...");
    fnf = sdd_dnf_read(options.dnf_filename);
    printf("vars=%"PRIlitS" terms=%"PRIsS"",fnf->var_count,fnf->litset_count);
  }
  
  if(options.vtree_filename!=NULL) {
    printf("\nreading initial vtree...");
    vtree = sdd_vtree_read(options.vtree_filename);
  } 
  else {
    printf("\ncreating initial vtree (%s)...",options.initial_vtree_type);
    vtree = sdd_vtree_new(fnf->var_count,options.initial_vtree_type);
  }
  
  printf("\ncreating manager...");
  //create manager
  cm = manager = sdd_manager_new(vtree);
  initialize_manager_search_state(manager);
  //no longer needed
  sdd_vtree_free(vtree);
  //passing compiler options to manager
  sdd_manager_set_options(&options,manager);

  printf("\ncompiling..."); fflush(stdout);
  c1 = clock();
  if(options.auto_minimize_and_gc) node = fnf_to_sdd_auto(fnf,manager);
  else node = fnf_to_sdd_manual(fnf,manager);
  c2 = clock();
  
  char* s;
  float secs = (float)(c2-c1)/CLOCKS_PER_SEC;
  printf("\n\ncompilation time        : %.3f sec\n",secs);
  printf(    " sdd size               : %s \n",s=ppc(sdd_size(node))); free(s);
  printf(    " sdd node count         : %s \n",s=ppc(sdd_count(node))); free(s);

  c1 = clock();
  SddModelCount mc = sdd_model_count(node,manager);
  c2 = clock();
  printf(    " sdd model count        : %s    %.3f sec\n",s=ppc(mc),(float)(c2-c1)/CLOCKS_PER_SEC); free(s);
  //printf(    "\n\n");

  sdd_manager_print(manager);
  
  if(options.minimize_cardinality) {
    printf("\nminimizing cardinality...");
    c1 = clock();
    node = sdd_minimize_cardinality(node,manager);
    c2 = clock();
    printf("size = %zu / node count = %zu / %.3f sec\n",sdd_size(node),sdd_count(node),(float)(c2-c1)/CLOCKS_PER_SEC);
  }
  
  Vtree* manager_vtree = sdd_manager_vtree(manager);

  if(options.vtree_search_mode==1) {
    sdd_ref(node,manager);
      printf("\ndynamic vtree (post compilation)\n");
      printf(    " sdd initial size       : %"PRIsS"\n",sdd_size(node));
      c1 = clock();
      vtree_search(manager_vtree,manager);
      c2 = clock();
      printf("\n");
      printf(    " dynamic vtree time     : %.3f sec\n",(float)(c2-c1)/CLOCKS_PER_SEC);
      printf(    " sdd size               : %"PRIsS"\n",sdd_size(node));
      printf(    " sdd node count         : %"PRIsS"\n",sdd_count(node));
      printf(    " sdd model count        : %"PRImcS"\n",sdd_model_count(node,manager));
    sdd_deref(node,manager);
  }

  if(options.output_sdd_filename != NULL) {
    printf("saving compiled sdd ...");
    sdd_save(options.output_sdd_filename,node);
    printf("done\n");
  }

  if(options.output_sdd_dot_filename != NULL) {
    printf("saving compiled sdd (dot)...");
    sdd_save_as_dot(options.output_sdd_dot_filename,node);
    printf("done\n");
  }

  if(options.output_vtree_filename != NULL) {
    printf("saving vtree...");
    sdd_vtree_save(options.output_vtree_filename,manager_vtree);
    printf("done\n"); 
  }

  if(options.output_vtree_dot_filename != NULL) {
    printf("saving vtree (dot)...");
    sdd_vtree_save_as_dot(options.output_vtree_dot_filename,manager_vtree);
    printf("done\n"); 
  }

  printf("freeing...");
  fflush(stdout);
  free_fnf(fnf);
  free_manager_search_state(manager);
  sdd_manager_free(manager);
  printf("done\n"); 

  return 0;
}

/****************************************************************************************
 * end
 ****************************************************************************************/
