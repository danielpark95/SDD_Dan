#!/usr/bin/env python

import math
import time
import glob
import sys

from pypsdd import *

# for printing numbers with commas
import locale
#locale.setlocale(locale.LC_ALL, "en_US.UTF8")

def model_str(model,n):
    """pretty print model"""

    keys = model.keys()
    keys.sort()
    st = []
    for i,key in enumerate(keys):
        val = str(model[key])
        if i > 0 and i % n == 0:
            st.append(',')
        st.append(val)
    return "".join(st)

if __name__ == '__main__':
    #basename = '32-3'
    #basename = 'fire_alarm'
    basename = 'parsings'
    vtree_filename = 'data/base/%s.vtree' % basename
    sdd_filename = 'data/base/%s.sdd' % basename

    psi,scale = 2.0,None # learning hyper-parameters
    N,M = 2**10,2**10 # size of training/testing dataset
    em_max_iters = 10 # maximum # of iterations for EM
    em_threshold = 1e-4 # convergence threshold
    seed = 1 # seed for simulating datasets

    ########################################
    # READ INPUT
    ########################################

    #print "== reading vtree/sdd"

    vtree = Vtree.read(vtree_filename)
    manager = SddManager(vtree)
    sdd = SddNode.read(sdd_filename,manager)
    pmanager = PSddManager(vtree)
    copy = pmanager.copy_and_normalize_sdd(sdd,vtree)
    pmanager.make_unique_true_sdds(copy,make_true=False) #AC: set or not set?

    #print "         sdd size: %d" % sdd.size()
    #print "           sdd nc: %d" % sdd.node_count()
    #print "        psdd size: %d" % copy.size()
    #print "          psdd nc: %d" % copy.node_count()
    #print "  psdd parameters: %d" % copy.theta_count()
    #print "       psdd trues: %d" % copy.true_count()

    for alpha in [sdd,copy]:
        start = time.time()
        model_count = alpha.model_count()
        #print "      model count: %s (%.3fs)" % \
            #(locale.format("%d",model_count,grouping=True),time.time()-start)

    ########################################
    # SIMULATE
    ########################################
    
    setCount = int(sys.argv[1])
    totalLL = 0
    location = sys.argv[2]
    
    for runCt in range (setCount):
    
        start = time.time()
        """
        copy.random_weights(psi=1.0) # set random weights on PSDD
        complete = DataSet.simulate(copy,N,seed=seed)
        #training = complete.hide_values_at_random(0.25,seed=seed)
        if type(seed) is int or type(seed) is long: seed = seed+1 # update seed
        testing  = DataSet.simulate(copy,M,seed=seed)
        """
        
        trainingName = location + "psddTraining" + str(runCt) + ".txt"
        training = DataSet.read(trainingName)
        #training = DataSet.read("../generateSet/psddTraining.txt")
        #testingName = "../generateSet/psddTesting" + str(runCt) + ".txt"
        testingName = location + "psddTesting" + str(runCt) + ".txt"
        #testing = DataSet.read("../generateSet/psddTesting.txt")
        testing = DataSet.read(testingName)

        """
        print "simulate datasets: %.3fs" % (time.time()-start)
        print "         training: %d unique, %d instances" % (len(training),training.N)
        print "          testing: %d unique, %d instances" % (len(testing),testing.N)
        """
        if type(seed) is int or type(seed) is long: seed = seed+1 # update seed

        ########################################
        # LEARN
        ########################################

        """
        # for complete data, for testing purposes
        start = time.time()
        copy.learn(complete,psi=psi,scale=scale,show_progress=True)
        print "    training time: %.3fs" % (time.time()-start)
        ll = copy.log_likelihood_alt(complete)
        lprior = copy.log_prior(psi=psi,scale=scale)
        print "   log likelihood: %.8f" % (ll/complete.N)
        print "    log posterior: %.8f" % ((ll+lprior)/complete.N)
        print "  zero parameters: %d (should be zero)" % copy.zero_count()
        copy.marginals()

        print "== TESTING =="
        ll = copy.log_likelihood_alt(testing)
        print "   log likelihood: %.8f" % (ll/testing.N)
        print "    log posterior: %.8f" % ((ll+lprior)/testing.N)
        """

        start = time.time()
        copy.random_weights(psi=2.0) # initial seed for EM
        stats = copy.soft_em(training,psi=psi,scale=scale,
                             threshold=em_threshold,max_iterations=em_max_iters)
        #ll = stats.ll
        #ll = copy.log_likelihood_alt(training)
        ll = copy.log_likelihood(training)
        lprior = copy.log_prior(psi=psi,scale=scale)
        #print "    training time: %.3fs (iters: %d)" % (time.time()-start,stats.iterations)
        #print "   log likelihood: %.8f" % (ll/training.N)
        #print "    log posterior: %.8f" % ((ll+lprior)/training.N)
        #print "  zero parameters: %d (should be zero)" % copy.zero_count()

        ########################################
        # TEST
        ########################################

        #print "== TESTING =="
        ll = copy.log_likelihood(testing)
        #print "   log likelihood: %.8f" % (ll/testing.N)
        totalLL = totalLL + ll/testing.N
        #print "    log posterior: %.8f" % ((ll+lprior)/testing.N)

    avgLL = totalLL/setCount
    #print "Average Log Likelihood: " + str(avgLL)
    out = open("logLikelihood.txt","w")
    out.write(str(avgLL))