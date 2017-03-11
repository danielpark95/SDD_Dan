This project represents context-free grammars as probabilistic logic bases using PSDDs.  It achieves this by representing a CFG canonically as a SDD and using machine learning to generate the PSDD.
The code to generate an SDD is located in sdd-1/parse.c.
  The parameters of the CFG are set at the top of the main function.  Don't worry about tmpNonCt and tmpTermCt, I used them in testing the size and node count of the SDD.
  The arrays nonRules and termRules hold the rules of the cfg, there are two ways to set them:
    1) You can input them to the translateRules function in the indicated manner and run translateRules with nonRules and termRules as arguments
      This function takes in the rules in the same form that generateDataset.c does, so it should probably only be used if you are using generateDataset.c to create your datasets.
    2) You can create separate arrays for non-terminal rules and terminal rules of the following forms:
      int utNonRules[6] = {000,111,12,202,221,201}
        If the non-terminals are 0:A, 1:B, 2:S, these equate to A -> AA, B -> BB, A -> BS (note a leading 0 is left off), S -> AS, etc.
      int utTermRules[2] = {00,11}
        If the non-terminals are the same as above and the terminals are 0:a, 1:b, these equate to A -> a, B -> b
				
      After setting these arrays, run transBoth(utNonRules,nonRules,nonRuleCount,utTermRules,termRules,termRuleCount) to input the rules to the nonRule and termRule arrays
			
  This script saves the sdd and vtree corresponding to the CFG
	
There are two shell scripts to run the learning experiments.
  master.sh runs learning experiments with pcfg generated datasets
    set the number of datasets that you want to take the average log likelihood over in the "count" parameter of master.sh
    set the set size of those datasets in the "setSize" parameter of master.sh
    Input the cfg to parse.c and generateDataset.c
    Input the rules to inside-outside/rules.txt
		
    You are now ready to run master.sh
		
		
  masterPsddGen.sh runs learning experiments with psdd generated datasets
    set the number of datasets that you want to take the average log likelihood over in the "count" parameter of masterPsddGen.sh
    set the set size of those datasets in the "setSize" parameter of masterPsddGen.sh
    Input the cfg to parse.c
    Input the rules to inside-outside/rules.txt
		
You are now ready to run masterPsddGen.sh
