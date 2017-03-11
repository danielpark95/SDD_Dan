#!/usr/bin/env python

import time
import math
import random
from collections import defaultdict
from collections import deque
from Queue import PriorityQueue as pq

from sdd import SddNode
from data import DataSet

class PSddNode(SddNode):

########################################
# CONSTRUCTOR
########################################

    def __init__(self,node_type,alpha,vtree,manager):
        SddNode.__init__(self,node_type,alpha,vtree,manager)
        self.negation = None
        self._init_theta(psi=1.0)
        self.is_false_sdd = False #AC?

    def initialize_parameters(self,psi=2.0,scale=None):
        if psi is not None:
            for node in self:
                node._init_theta(psi=psi)
        elif scale is not None:
            self.uniform_weights(scale=scale)

    def _init_theta(self,psi=2.0):
        """psi is Dirichlet parameter"""

        count = psi-1.0 # psedo-count
        if self.is_false():
            pass
        elif self.is_true():
            self.theta = [count,count]
            self.theta_sum = 2.0*count
        elif self.is_literal():
            self.theta = [0.0,count] if self.literal > 0 else [count,0.0]
            self.theta_sum = count
        elif self.is_decomposition():
            self.theta = dict()
            self.theta_sum = 0.0
            for element in self.elements:
                prime,sub = element
                if sub.is_false_sdd:
                    self.theta[element] = 0.0
                else:
                    self.theta[element] = count
                    self.theta_sum += count

    def uniform_weights(self,scale=1.0):
        """uniform prior over SDD models"""
        self.linearize()

        self.initialize_parameters(psi=1.0)
        self.model_count(clear_data=False)
        for node in self.array: node.data = float(node.data)
        self.theta_sum = float(scale)
        for node in reversed(self.array):
            if node.is_false() or node.is_false_sdd: continue
            # if node.theta_sum == 0.0:
            #     # unused node (prime of false sub)
            #     node.theta_sum = 1.0 # ACAC for Dirichlet
            count = node.theta_sum
            if node.is_true():
                node.theta = [count/2.0,count/2.0]
            elif node.is_literal():
                node.theta = [0.0,count] if node.literal > 0 else [count,0.0]
            else: # node.is_decomposition()
                for element in node.elements:
                    prime,sub = element
                    if sub.is_false_sdd: continue
                    uniform_theta = prime.data*sub.data/node.data
                    decision_count = count * uniform_theta
                    node.theta[element] = decision_count
                    prime.theta_sum += decision_count
                    sub.theta_sum += decision_count

        for node in self.array: node.data = None

########################################
# RANDOM PARAMETERIZATION
########################################

    @staticmethod
    def random_pr(k,psi=1.0):
        """k is number of states"""

        pr = [ random.gammavariate(psi,1) for i in xrange(k) ]
        #pr = [ -math.log(random.random()) for i in xrange(k) ]
        pr_sum = sum(pr)
        pr = [ p/pr_sum for p in pr ]
        return pr

    def random_weights(self,psi=1.0):
        """for each decision node, sample a multinomial distribution from a
        flat Dirichlet prior"""

        self.initialize_parameters(psi=1.0)
        for node in self:
            if node.is_false():
                pass
            elif node.is_true():
                node.theta = PSddNode.random_pr(2,psi=psi)
                node.theta_sum = 1.0
            elif node.is_literal():
                node.theta = [0.0,1.0] if node.literal > 0 else [1.0,0.0]
                node.theta_sum = 1.0
            else: # node.is_decomposition()
                count = 0
                for prime,sub in node.elements:
                    if not sub.is_false_sdd: count += 1
                pr = PSddNode.random_pr(count,psi=psi)
                iter_pr = iter(pr)
                for element in node.elements:
                    prime,sub = element
                    if sub.is_false_sdd:
                        continue
                        # node.theta[element] = 0.0
                    else:
                        node.theta[element] = iter_pr.next()
                node.theta_sum = 1.0

    def scale_weights(self,scale):
        for node in self:
            if node.is_false():
                pass
            elif node.vtree.is_leaf():
                node.theta[0] = scale*node.theta[0]
                node.theta[1] = scale*node.theta[1]
                node.theta_sum = scale*node.theta_sum
            else: # node.is_decomposition()
                for element in node.elements:
                    prime,sub = element
                    if sub.is_false_sdd: continue
                    node.theta[element] = scale*node.theta[element]
                node.theta_sum = scale*node.theta_sum

########################################
# UTIL
########################################

    @staticmethod
    def instance_lit(instance,var):
        return var if instance[var-1] else -var

    @staticmethod
    def instance_models_lit(instance,lit):
        var = abs(lit)
        instance_lit = var if instance[var-1] else -var
        return lit == instance_lit

########################################
# STATS
########################################

    def _theta_count(self):
        """helper function"""
        if self.bit is True: return 0
        self.bit = True

        count = 0
        if self.is_true():
            count = 1
        elif self.is_decomposition():
            for element in self.elements:
                prime,sub = element
                if sub.is_false_sdd: continue
                count += 1
                count += prime._theta_count()
                count += sub._theta_count()
            count -= 1

        return count

    def theta_count(self):
        """counts the number of free parameters in a PSDD"""
        count = self._theta_count()
        self.clear_bits()
        return count

    def zero_count(self):
        """counts the number of zeros in a PSDD
           (not just reachable zeros, as in true count)"""
        self.linearize()
        count = 0
        for node in self.array:
            if node.is_false():
                pass
            elif node.is_true():
                if node.theta[0] == 0: count += 1
                if node.theta[1] == 0: count += 1
            elif node.is_literal():
                if node.theta[ node.literal > 0 ] == 0: count += 1
            else:
                for element in node.elements:
                    prime,sub = element
                    if sub.is_false_sdd: continue
                    if node.theta[element] == 0: count += 1
        return count

    def _true_count(self):
        """helper function"""
        if self.bit is True: return 0
        self.bit = True

        count = 0
        if self.is_true():
            count = 1
        elif self.is_decomposition():
            for element in self.elements:
                prime,sub = element
                if sub.is_false_sdd: continue
                count += prime._true_count()
                count += sub._true_count()

        return count

    def true_count(self):
        """counts the number of true nodes in a PSDD"""
        count = self._true_count()
        self.clear_bits()
        return count

########################################
# QUERIES
########################################

    def model_count(self,evidence={},clear_data=True):
        """model count"""
        self.linearize()
        for node in self.array:
            if node.is_false():
                count = 0
            elif node.is_true():
                #count = 2
                count = 1 if node.vtree.var in evidence else 2
            elif node.is_literal():
                #count = 1
                var = node.vtree.var
                if var in evidence:
                    val = 1 if node.literal > 0 else 0
                    count = 1 if evidence[var] == val else 0
                else:
                    count = 1
            else: # node.is_decomposition()
                count = 0
                for prime,sub in node.elements:
                    count += prime.data*sub.data
            node.data = count

        if clear_data:
            for node in self.array: node.data = None
        return count

    def marginals(self,evidence={},clear_data=True,do_bottom_up=True):
        """does top-down pass for marginals"""
        #self.linearize() # redundant:
        self._initialize_marginals()

        if do_bottom_up: self.value(evidence=evidence,clear_data=False)
        value = self.data

        self.pr_context = 1.0
        for node in reversed(self.array):
            if node.is_false_sdd:
                pass
            else:
                if node.theta_sum == 0.0: continue
                if node.is_decomposition():
                    for element in node.elements:
                        prime,sub = element
                        if sub.is_false_sdd: continue
                        theta = node.theta[element]/node.theta_sum
                        pr_prime = prime.data/prime.theta_sum
                        pr_sub = sub.data/sub.theta_sum
                        prime.pr_context += theta*pr_sub*node.pr_context
                        sub.pr_context += theta*pr_prime*node.pr_context
                node.pr_node = node.pr_context*(node.data/node.theta_sum)

        if clear_data: self.clear_data()
        return value

    def all_marginals(self,evidence={}):
        self.marginals(evidence)
        vtree2marginals = defaultdict(lambda : [])
        for node in self.array:
            vtree2marginals[node.vtree].append(node.pr_node)
        for vtree in vtree2marginals:
            vtree2marginals[vtree].sort(cmp=lambda x,y: -cmp(x,y))
        return vtree2marginals

    def _initialize_marginals(self):
        self.linearize()
        for node in self.array:
            node.pr_context = 0.0
            node.pr_node = 0.0

########################################
# SIMULATE A PSDD
########################################

    def sample(self,pr,z=1.0):
        """draws a sample from a discrete distribution pr = [p_1,...,p_n]
        assumes pr is an iterable of tuples (item,probability)"""

        q = random.random()
        cur = 0
        for item,p in pr:
            cur += p/z
            if q <= cur:
                return item
        return item

    def simulate(self,seed=None):
        """simulate a model"""

        if seed: random.seed(seed)

        instance = [None]*self.vtree.var_count
        self._simulate(instance)
        # for i,val in enumerate(instance):
        #     if val is None:
        #         print "error: variable %d was not set to a value" % (var+1)
        #         raise Exception

        return instance

    def _simulate(self,instance):
        if self.is_false(): # should not happen
            pass
        elif self.is_true():
            var = self.vtree.var
            p = self.theta[0]/self.theta_sum
            val = 0 if random.random() < p else 1
            instance[var-1] = val
        elif self.is_literal():
            var = self.vtree.var
            lit = self.literal
            val = 0 if lit < 0 else 1
            instance[var-1] = val
        else:
            pr = self.theta.iteritems()
            element = self.sample(pr,z=self.theta_sum)
            prime,sub = element
            prime._simulate(instance)
            sub._simulate(instance)

########################################
# LEARN (BOTTOM-UP/TOP-DOWN)
########################################

    def learn_two_pass(self,dataset,psi=2.0,scale=None):
        """learn the parameters of a psdd from a complete dataset"""

        self.initialize_parameters(psi=psi,scale=scale)
        for instance,count in dataset:
            count = float(count)
            self.learn_bottom_up_pass(instance)
            self.learn_top_down_pass(instance,count)
        self.clear_data()

    def learn_bottom_up_pass(self,instance):
        """bottom-up: we mark nodes consistent with instance.

        for decompositions, we mark the consistent element (the decision),
        otherwise it is not None
        """

        for node in self:
            node.data = None # un-mark
            if node.is_false():
                pass
            elif node.is_true():
                node.data = node
            elif node.is_literal():
                if PSddNode.instance_models_lit(instance,node.literal):
                    node.data = node
            else: # node.is_decomposition()
                for prime,sub in node.elements:
                    if prime.data and sub.data:
                        node.data = (prime,sub)
                        break

    def learn_top_down_pass(self,instance,count):
        """top-down: we follow the marked edges (decisions) and increment
        the counts on edges and nodes"""

        if self.data is None:
            print "error: in top-down pass, data instance was inconsistent"
            raise Exception
        if self.is_false():
            pass
        elif self.vtree.is_leaf(): # true or literal
            lit = PSddNode.instance_lit(instance,self.vtree.var)
            self.theta[ lit > 0 ] += count
            self.theta_sum += count
        else:
            prime,sub = element = self.data
            self.theta[element] += count
            self.theta_sum += count

            # recurse
            prime.learn_top_down_pass(instance,count)
            sub.learn_top_down_pass(instance,count)

########################################
# LEARN (FOLLOW-DECISION)
########################################

    def learn(self,dataset,psi=None,scale=1.0,show_progress=True):
        """learn PSDD parameters from a complete dataset"""
        self.initialize_parameters(psi=psi,scale=scale)
        instance_count = len(dataset)
        for i,(instance,count) in enumerate(dataset):
            if show_progress:
                if instance_count >= 10 and i % (instance_count/10) == 0:
                    print "%2.0f%% done" % (100*i/instance_count)
            count = float(count)
            self._mark_decisions(instance)
            self.learn_top_down_pass(instance,count)
            self.clear_bits()
        self.clear_data()

    def _mark_decisions(self,instance):
        if self.bit is True: return
        self.bit = True
        self.data = None

        if self.is_false():
            pass
        elif self.is_true():
            self.data = self
        elif self.is_literal():
            if PSddNode.instance_models_lit(instance,self.literal):
                self.data = self
        else: # self.is_decomposition()
            for prime,sub in self.elements:
                prime._mark_decisions(instance)
                if prime.data is None: continue
                sub._mark_decisions(instance)
                if sub.data is None: continue
                self.data = (prime,sub)
                break

########################################
# INFERENCE
########################################

    def value(self,evidence={},clear_data=True):
        """bottom-up evaluation of PSDD.
           evidence is map: var to val"""
        self.linearize()

        for node in self.array:
            if node.is_false():
                node.data = 0.0
            elif node.vtree.is_leaf():
                var = node.vtree.var
                if var in evidence:
                    val = evidence[var]
                    node.data = node.theta[val]
                else: node.data = node.theta_sum
            else: # decomposition
                value = 0.0
                for element in node.elements:
                    prime,sub = element
                    if sub.is_false_sdd: continue
                    # AC: is there a better test for the following?
                    if prime.theta_sum == 0 or sub.theta_sum == 0: continue
                    p_value = prime.data/prime.theta_sum
                    s_value = sub.data/sub.theta_sum
                    value += p_value * s_value * node.theta[element]
                node.data = value
        value = node.data

        if clear_data:
            for node in self.array: node.data = None
        return value

    def probability(self,evidence={}):
        return self.value(evidence)/self.theta_sum

    def log_likelihood(self,dataset):
        ll = 0.0
        for instance,count in dataset:
            evidence = DataSet.evidence(instance)
            pr = self.probability(evidence)
            ll += count * math.log(pr)
        return ll

    def value_top_down(self,instance):
        """top-down: we follow the marked edges (decisions) and evaluate"""
        if self.data is None:
            #print "error: in top-down pass, data instance was inconsistent"
            #raise Exception
            return 0.0
        if self.is_false():
            return 0.0
        elif self.vtree.is_leaf(): # true or literal
            lit = PSddNode.instance_lit(instance,self.vtree.var)
            return self.theta[ lit > 0 ]
        else:
            p,s = element = self.data
            if p.theta_sum == 0.0 or s.theta_sum == 0.0: return 0.0
            p_value = p.value_top_down(instance)/p.theta_sum
            s_value = s.value_top_down(instance)/s.theta_sum
            return p_value * s_value * self.theta[element]

    def log_likelihood_alt(self,dataset):
        """faster version, for complete datasets"""
        ll = 0.0
        for instance,count in dataset:
            self._mark_decisions(instance)
            pr = self.value_top_down(instance)/self.theta_sum
            if pr == 0.0:
                print "error: instance has zero probability"
                raise Exception
            ll += count * math.log(pr)
            self.clear_bits()
        self.clear_data()
        return ll

    def log_likelihood_list(self,dataset):
        """list of log likelihoods, for complete datasets"""
        lls = []
        for instance,count in dataset:
            self._mark_decisions(instance)
            pr = self.value_top_down(instance)/self.theta_sum
            if pr == 0.0:
                print "error: instance has zero probability"
                raise Exception
            lls.append(math.log(pr))
            self.clear_bits()
        self.clear_data()
        return lls

    def _list_top_down(self,instance):
        if self.is_false():
            return [0.0]
        elif self.vtree.is_leaf(): # true or literal
            lit = PSddNode.instance_lit(instance,self.vtree.var)
            return [self.theta[ lit > 0 ]/self.theta_sum]
        else:
            p,s = element = self.data
            #if p.theta_sum == 0.0 or s.theta_sum == 0.0: return [0.0]
            p_list = p._list_top_down(instance)
            s_list = s._list_top_down(instance)
            return p_list + s_list + [ self.theta[element]/self.theta_sum ]

    def parameter_list(self,instance):
        """returns a flat list of all PSDD parameters"""
        self._mark_decisions(instance)
        plist = self._list_top_down(instance)
        #log_pr = 0.0
        #for p in plist:
        #    log_pr += math.log(p)
        self.clear_bits()
        self.clear_data()
        return plist

########################################
# SOFT EM LEARNING (BOTTOM-UP/TOP-DOWN)
########################################

    def backup_thetas(self):
        self.linearize()
        for node in self.array:
            if node.is_false():
                pass
            elif node.vtree.is_leaf():
                node.backup_theta = list(node.theta)
                node.backup_theta_sum = node.theta_sum
            else: # decomposition
                node.backup_theta = dict(node.theta)
                node.backup_theta_sum = node.theta_sum

    def swap_thetas(self):
        self.linearize()
        for node in self.array:
            if node.is_false():
                pass
            else:
                node.theta,node.backup_theta = node.backup_theta,node.theta
                node.theta_sum,node.backup_theta_sum = \
                    node.backup_theta_sum,node.theta_sum

    def log_prior(self,psi=2.0,scale=None,swap_back=True):
        if psi is not None:
            log_prior = 0.0
            for node in self:
                if node.is_false(): pass
                elif node.is_literal(): pass
                elif node.theta_sum == 0.0: pass #ACAC
                elif node.is_true():
                    for theta in node.theta:
                        theta = theta/node.theta_sum
                        log_prior += (psi-1.0) * math.log(theta)
                elif node.is_decomposition():
                    for element in node.elements:
                        if element[1].is_false_sdd: continue
                        theta = node.theta[element]/node.theta_sum
                        log_prior += (psi-1.0) * math.log(theta)
        elif scale is not None:
            """scale: \sum_w log P(w) = -mc * [kl + log mc]"""
            mc = self.model_count()
            self.backup_thetas()
            self.uniform_weights(scale=scale) # scale shouldn't matter here
            self.marginals()
            kl = self.kl_psdd()
            if swap_back: self.swap_thetas()
            log_prior = -scale * (kl+math.log(mc))

        return log_prior

    class EmStats:
        def __init__(self,threshold,max_iterations,N=1):
            self.threshold = threshold
            self.max_iterations = max_iterations
            self.last_ll = None
            self.ll = None
            self.residual = None
            self.iterations = 0
            self.N = N
            self.train_time = None
            self.timer = time.time()
            self.data_times = []
            self.pool_times = []
            self.iter_time = None

        def converged(self):
            if self.iterations <= 1:
                return False
            elif self.iterations > self.max_iterations:
                return True
            else:
                return self.residual < self.threshold

        def start_iteration(self):
            self.last_ll,self.ll = self.ll,0.0
            self.data_times = []
            self.pool_times = []
            self.iter_time = time.time()

        def incr_likelihood(self,incr):
            self.ll += incr

        def end_iteration(self):
            self.iterations += 1
            self.train_time = time.time()-self.timer
            self.iter_time = time.time()-self.iter_time
            if self.iterations <= 1: return
            #self.residual = (self.ll-self.last_ll)/abs(self.ll)
            self.residual = (self.ll-self.last_ll)/self.N
            if self.residual < -1e-12:
                print "error: in EM, likelihood fell"
                #raise Exception

        def print_iteration(self):
            print "iteration %d: ll = %.8f" % (self.iterations,self.ll/self.N),
            print "[%.3fs]" % (time.time()-self.iter_time),
            if self.pool_times:
                times = self.pool_times
                data_time = sum(self.data_times)/len(self.data_times) if \
                            len(self.data_times) > 0 else 0.0
                
                print "(avg: %.1fs data: %.1fs max: %.1fs min: %.1fs sum: %.1fs)" % \
                    (sum(times)/len(times),data_time, \
                     max(times),min(times),sum(times)),
            print

    """assumes current thetas is a seed"""
    def soft_em(self,dataset,psi=2.0,scale=None,
                threshold=1e-4,max_iterations=10,e_step_f=None):
        stats = PSddNode.EmStats(threshold,max_iterations,N=dataset.N)
        self.backup_thetas()
        if e_step_f is None: e_step_f = self.soft_em_e_step

        while not stats.converged():
            stats.start_iteration()
            #self.initialize_parameters(psi=psi,scale=scale)
            #self.swap_thetas()

            if scale is not None:
                self.swap_thetas()
                stats.ll = self.log_prior(psi=psi,scale=scale,swap_back=False)
                self.swap_thetas()
            elif psi is not None:
                self.initialize_parameters(psi=psi,scale=scale)
                self.swap_thetas()
                stats.ll = self.log_prior(psi=psi,scale=scale)

            # E-step
            for instance,count in dataset:
                pre = e_step_f(instance,count=count)
                stats.incr_likelihood(count * math.log(pre))

            # M-step: do nothing

            stats.print_iteration()
            stats.end_iteration()

        self.clear_data()
        return stats

    def soft_em_e_step(self,instance,count=1):
        count,evidence = float(count),DataSet.evidence(instance)
        value = self.value(evidence=evidence,clear_data=False)
        pre = value/self.theta_sum
        self.marginals(evidence=evidence,clear_data=False,do_bottom_up=False)
        self._soft_em_accumulate(evidence,pre,count=count)
        return pre

    def _soft_em_accumulate(self,evidence,pre,count=1):
        for node in reversed(self.array):
            if node.is_false(): continue
            if node.is_false_sdd: continue
            if node.pr_context == 0.0: continue
            if node.theta_sum == 0.0: continue # ACACAC

            if node.vtree.is_leaf(): # true or literal
                var = node.vtree.var
                vals = [evidence[var]] if var in evidence else [0,1]
                for val in vals:
                    pr = (node.theta[val]/node.theta_sum)*node.pr_context
                    node.backup_theta[val] += count*(pr/pre)
                    node.backup_theta_sum  += count*(pr/pre)
            else: # node.is_decomposition():
                for element in node.elements:
                    prime,sub = element
                    if sub.is_false_sdd: continue
                    theta = node.theta[element]/node.theta_sum
                    prime_evd = prime.data/prime.theta_sum
                    sub_evd = sub.data/sub.theta_sum
                    pr = prime_evd * sub_evd * theta * node.pr_context
                    node.backup_theta[element] += count*(pr/pre)
                    node.backup_theta_sum += count*(pr/pre)
