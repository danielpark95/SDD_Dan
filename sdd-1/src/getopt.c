/****************************************************************************************
 * The Sentential Decision Diagram Package
 * sdd version 1.1.1, January 31, 2014
 * http://reasoning.cs.ucla.edu/sdd
 ****************************************************************************************/

#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "sddapi.h"
#include "compiler.h"
#include "parameters.h"

void print_help(const char* PACKAGE, int exit_value);
const char* sdd_version();

/****************************************************************************************
 * start
 ****************************************************************************************/
 
SddCompilerOptions sdd_getopt(int argc, char **argv) {
  char* PACKAGE = SDD_PACKAGE;

  //default options 
  SddCompilerOptions options = 
    {
    NULL,NULL,NULL,NULL,NULL,NULL,NULL, //file names
    1,0, //flags
    INITIAL_VTREE,
    GC_THRESHOLD,
    VTREE_SEARCH_THRESHOLD,
    VTREE_SEARCH_MODE
    };
  int option;
     
  while ((option = getopt(argc,argv,"c:d:v:W:V:R:S:hmMt:g:n:r:")) != -1) {
    switch (option) {
    //input
    case 'c':
      options.cnf_filename = optarg;
      break;
    case 'd':
      options.dnf_filename = optarg;
      break;
    case 'v':
      options.vtree_filename = optarg;
      break;
    //output
    case 'W':
      options.output_vtree_filename = optarg;
      break;
    case 'V':
      options.output_vtree_dot_filename = optarg;
      break;
    case 'R':
      options.output_sdd_filename = optarg;
      break;
    case 'S':
      options.output_sdd_dot_filename = optarg;
      break;
    //flags
    case 'h': //HELP
      print_help(PACKAGE,0);
      break;
    case 'M': 
      options.auto_minimize_and_gc = 0;
      break;
    case 'm': 
      options.minimize_cardinality = 1;
      break;
    //options with arguments
    case 't':
      options.initial_vtree_type = optarg;
      break;
    case 'g': 
      options.gc_threshold = strtof(optarg,NULL);
      break;
    case 'n': 
      options.vtree_search_threshold = strtof(optarg,NULL);
      break;
    case 'r':
      options.vtree_search_mode = strtol(optarg,NULL,10);
      break;
    default:
      print_help(PACKAGE,1);
    }
  }

  //checking validity of options  
  if(options.cnf_filename==NULL && options.dnf_filename==NULL) {
    fprintf(stderr, "%s: must specify a cnf or dnf file\n",PACKAGE);
    print_help(PACKAGE,1);
  }
  if(options.cnf_filename!=NULL && options.dnf_filename!=NULL) {
    fprintf(stderr, "%s: cannot specify both cnf and dnf files (only one)\n",PACKAGE);
    print_help(PACKAGE,1);
  }
  if(options.vtree_search_mode < 0 || options.vtree_search_mode > 2) {
    fprintf(stderr, "%s: option -r must be 0, 1 or 2\n",PACKAGE);
    print_help(PACKAGE,1);
  }
  if(strcmp(options.initial_vtree_type,"left") && 
     strcmp(options.initial_vtree_type,"right") &&
     strcmp(options.initial_vtree_type,"vertical") && 
     strcmp(options.initial_vtree_type,"balanced")) {
   fprintf(stderr, "%s: initial vtree type must be one of: left, right, vertical, or balanced\n",PACKAGE);
    print_help(PACKAGE,1);
  } 
 
  return options;
}


void print_help(const char* PACKAGE, int exit_value) {
  printf("%s: Sentential Decision Diagram, Compiler\n", PACKAGE);
  printf("%s\n",sdd_version());
  printf("%s [-c .] [-d .] [-v .]   [-W .] [-V .] [-R .] [-S .]   [-h] [-M] [-m]  [-t]  [-g .] [-n .] [-r .]\n", PACKAGE);

  printf("  -c FILE         set input CNF file\n"); 
  printf("  -d FILE         set input DNF file\n");  
  printf("  -v FILE         set input VTREE file\n");
    
  printf("  -W FILE         set output VTREE file\n");
  printf("  -V FILE         set output VTREE (dot) file\n");
  printf("  -R FILE         set output SDD file\n");
  printf("  -S FILE         set output SDD (dot) file\n");
 
  printf("  -h              print this help and exit\n");
  printf("  -M              use manual sdd-minimize and gc\n");
  printf("  -m              minimize the cardinality of compiled sdd\n");
  
  printf("  -t TYPE         set initial vtree type (left, right, vertical, or balanced)\n");
  
  printf("  -g THR          set threshold for invoking garbage collection\n");
  printf("  -n THR          set threshold for invoking dynamic vtree search\n");
  printf("  -r MODE         set vtree search mode:\n");
  printf("                    0: no vtree search\n");
  printf("                    1: post-compilation vtree search\n");
  printf("                    2: dynamic vtree search (default)\n");
  exit(exit_value);
}

const char* sdd_version() {
  return SDD_PACKAGE " version " SDD_VERSION ", " SDD_DATE;
}

/****************************************************************************************
 * end
 ****************************************************************************************/
