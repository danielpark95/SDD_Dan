#!/usr/bin/env python

#this script generates a given umber of psdd training and testing sets

import math
import time
import glob

from pypsdd import *

# for printing numbers with commas
#import locale
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
    vtree_filename = '../psdd/data/base/%s.vtree' % basename
    sdd_filename = '../psdd/data/base/%s.sdd' % basename

    #vtree_filename = 'data/base/%s.vtree' % basename
    #sdd_filename = 'data/base/%s.sdd' % basename

    psi,scale = 2.0,None # learning hyper-parameters
    N,M = 2**10,2**10 # size of training/testing dataset
    em_max_iters = 10 # maximum # of iterations for EM
    em_threshold = 1e-4 # convergence threshold
    seed = 1 # seed for simulating datasets

    ########################################
    # READ INPUT
    ########################################

    print "== reading vtree/sdd"

    vtree = Vtree.read(vtree_filename)
    manager = SddManager(vtree)
    sdd = SddNode.read(sdd_filename,manager)
    pmanager = PSddManager(vtree)
    copy = pmanager.copy_and_normalize_sdd(sdd,vtree)
    pmanager.make_unique_true_sdds(copy,make_true=False) #AC: set or not set?

    print "         sdd size: %d" % sdd.size()
    print "           sdd nc: %d" % sdd.node_count()
    print "        psdd size: %d" % copy.size()
    print "          psdd nc: %d" % copy.node_count()
    print "  psdd parameters: %d" % copy.theta_count()
    print "       psdd trues: %d" % copy.true_count()

    #for alpha in [sdd,copy]:
     #   start = time.time()
      #  model_count = alpha.model_count()
       # print "      model count: %s (%.3fs)" % \
        #    (locale.format("%d",model_count,grouping=True),time.time()-start)

    ########################################
    # SIMULATE
    ########################################
    
    numSets = 1
    length = 7
    termCount = 2
    termLits = length*termCount
    start = time.time()
    copy.random_weights(psi=1.0) # set random weights on PSDD
    for i in range (numSets):
        complete = DataSet.simulate(copy,N,seed=seed)
        trainingName = "psddGeneratedSets/psddTraining.txt"
        fTrain = open(trainingName,'w')
        for instance,count in complete:
            #print count,instance
            for m in range(count):
                index = 0
                for char in instance:
                    if index >= termLits:
                        fTrain.write("-1")
                    else:
                        fTrain.write(str(char))
                    if index < len(instance)-1:
                        fTrain.write(',')
                    index += 1
                fTrain.write('\n')
        fTrain.close()
	
        if type(seed) is int or type(seed) is long: seed = seed+1 # update seed
        testing  = DataSet.simulate(copy,M,seed=seed)
        testingName = "psddGeneratedSets/psddTesting.txt"
        fTest = open(testingName,'w')
        for instance,count in testing:
            #print count,instance
            for m in range(count):
                index = 0
                for char in instance:
                    if index >= termLits:
                        fTest.write("-1")
                    else:
                        fTest.write(str(char))
                    if index < len(instance)-1:
                        fTest.write(',')
                    index += 1
                fTest.write('\n')
        fTest.close()
        if type(seed) is int or type(seed) is long: seed = seed+1 # update seed
    
    """
    training = complete.hide_values_at_random(0.25,seed=seed)
    if type(seed) is int or type(seed) is long: seed = seed+1 # update seed
    testing  = DataSet.simulate(copy,M,seed=seed)
    print "simulate datasets: %.3fs" % (time.time()-start)
    print "         training: %d unique, %d instances" % (len(training),training.N)
    print "          testing: %d unique, %d instances" % (len(testing),testing.N)
    if type(seed) is int or type(seed) is long: seed = seed+1 # update seed

    """

    ########################################
    # LEARN
    ########################################

    start = time.time()
    #trainingName = "../"
    training = DataSet.read(trainingName)
    testing = DataSet.read(testingName)

    start = time.time()
    copy.random_weights(psi=2.0) # initial seed for EM
    stats = copy.soft_em(training,psi=psi,scale=scale,
                         threshold=em_threshold,max_iterations=em_max_iters)

    ll = copy.log_likelihood(training)
    lprior = copy.log_prior(psi=psi,scale=scale)

    ll = copy.log_likelihood(testing)
    print ll/testing.N


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

    """
    start = time.time()
    copy.random_weights(psi=1.0) # initial seed for EM
    stats = copy.soft_em(training,psi=psi,scale=scale,
                         threshold=em_threshold,max_iterations=em_max_iters)
    #ll = stats.ll
    #ll = copy.log_likelihood_alt(training)
    ll = copy.log_likelihood(training)
    lprior = copy.log_prior(psi=psi,scale=scale)
    print "    training time: %.3fs (iters: %d)" % (time.time()-start,stats.iterations)
    print "   log likelihood: %.8f" % (ll/training.N)
    print "    log posterior: %.8f" % ((ll+lprior)/training.N)
    print "  zero parameters: %d (should be zero)" % copy.zero_count()

    ########################################
    # TEST
    ########################################

    print "== TESTING =="
    ll = copy.log_likelihood_alt(testing)
    print "   log likelihood: %.8f" % (ll/testing.N)
    print "    log posterior: %.8f" % ((ll+lprior)/testing.N)

    """