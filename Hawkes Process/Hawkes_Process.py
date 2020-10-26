""" Hawkes Process """

#Date : 24/10/2020
#Auteur : Team 13 SDI (Charlotte A., Simon H. Pierrick H.)
#Objectif : Obtenir les paramètres d'un processus de Hawkes en utilisant la vraissemblance

#----------- Import -----------#

import numpy as np
import scipy.optimize as optim
from kafka import KafkaProducer
from json import dumps

#----------- Programme --------#

#------ Fonctions Principales ---#

def MAP(history, t, alpha, mu, prior_params = [ 0.02, 0.0002, 0.01, 0.001, -0.1], max_n_star = 1):
    
    mu_p, mu_beta, sig_p, sig_beta, corr = prior_params
    sample_mean = np.array([mu_p, mu_beta])
    cov_p_beta = corr * sig_p * sig_beta
    Q = np.array([[sig_p ** 2, cov_p_beta], [cov_p_beta, sig_beta **2]])
    
    cov_prior = np.log(Q / sample_mean.reshape((-1,1)) / sample_mean.reshape((1,-1)) + 1)
    mean_prior = np.log(sample_mean) - np.diag(cov_prior) / 2.

    inv_cov_prior = np.asmatrix(cov_prior).I

    # Define the target function to minimize as minus the log of the a posteriori density    
    def target(params):
        log_params = np.log(params)
        
        if np.any(np.isnan(log_params)):
            return np.inf
        else:
            dparams = np.asmatrix(log_params - mean_prior)
            prior_term = float(- 1/2 * dparams * inv_cov_prior * dparams.T)
            logLL = loglikelihood(params, history, t)
            return - (prior_term + logLL)
      
    EM = mu * (alpha - 1) / (alpha - 2)
    eps = 1.E-8

    p_min, p_max       = eps, max_n_star/EM - eps
    beta_min, beta_max = 1/(3600. * 24 * 10), 1/(60. * 1)
    
    bounds = optim.Bounds(
        np.array([p_min, beta_min]),
        np.array([p_max, beta_max])
    )
    
    res = optim.minimize(
        target, sample_mean,
        method='Powell',
        bounds=bounds,
        options={'xtol': 1e-8, 'disp': False}
    )

    return(-res.fun, res.x) #On obtiens les paramètres qui maximise la vraisemblance

def loglikelihood(params,history,t):
    p,beta = params    
    
    if p <= 0 or p >= 1 or beta <= 0.: return -np.inf

    n = len(history)
    tis = history[:,0]
    mis = history[:,1]
    
    LL = (n-1) * np.log(p * beta)
    logA = -np.inf
    prev_ti, prev_mi = history[0]
    
    i = 0
    for ti,mi in history[1:]:
        if(prev_mi + np.exp(logA) <= 0):
            print("Bad value", prev_mi + np.exp(logA))
        
        logA = np.log(prev_mi + np.exp(logA)) - beta * (ti - prev_ti)
        LL += logA
        prev_ti,prev_mi = ti,mi
        i += 1
        
    logA = np.log(prev_mi + np.exp(logA)) - beta * (t - prev_ti)
    LL -= p * (np.sum(mis) - np.exp(logA))

    return LL
    

def prediction(params, history, alpha, mu, t):
    p,beta = params
    
    nstar = p*mu*((alpha-1)/(alpha-2))
    n = len(history)
    Somme = 0
    
    for t_i,m_i in history:
        if t_i < t:
            Somme = Somme + m_i*np.exp(-beta*(t-t_i))
    Somme = p*Somme
    
    N = n+(Somme/(1-nstar))
    
    return(N)

#------ Fonctions de vérification ------#

def simulate_marked_exp_hawkes_process(params, m0, alpha, mu, max_size=10000):

    p, beta = params    
    
    # Every row contains a marked time point (ti,mi).
    # Create an unitialized array for optimization purpose (only memory allocation)
    T = np.empty((max_size,2))
    
    intensity = beta * p * m0
    t, m = 0., m0
    
    # Main loop
    for i in range(max_size):
        # Save the current point before generating the next one.
        T[i] = (t,m)
        
        # 
        u = np.random.uniform()
        v = -np.log(u)
        w = 1. - beta / intensity * v
        # Tests if process stops generating points.
        if w <= 0.:
            T = T[:i,:]
            break
            
        # Otherwise computes the time jump dt and new time point t
        dt = - np.log(w) / beta
        t += dt
        
        # And update intensity accordingly
        m = neg_power_law(alpha, mu)
        lambda_plus = p * m
        intensity = intensity * np.exp(-beta * dt) + beta * lambda_plus        
    return(T)

def neg_power_law(alpha, mu, size=1):
    
    u = np.random.uniform(size=size)
    
    m = np.empty((size,1))
    
    for k in range(size):
        m[k] = mu*((1-u[k])**(1/(1-alpha)))
    return(m)

def test_MLE():
    p = 0.02
    beta = 1/3600 #2.77778e-4
    params = (p,beta)
    m0 = 1000
    alpha = 2.4
    mu = 10
    cascade = simulate_marked_exp_hawkes_process(params, m0, alpha, mu, max_size=10000)
    t = cascade[-1,0]
    
    res = MLE(cascade,t,alpha,mu,init_params=np.array([0.0001,1/60]))
    p_est,beta_est = res[1]

    print('La valeur entré de p est : ',p)
    print('La valeur estimé de p est : ',p_est) 
    print('La valeur entré de beta est : ',beta)
    print('La valeur estimé de beta est : ',beta_est)

#--------- Partie KAFKA ------#
### A modifier pour prendre en compte les vraies cascades
producer = KafkaProducer(bootstrap_servers='localhost:9092')
p = 0.02
beta = 1/3600 #2.77778e-4
params = (p,beta)
m0 = 1000
alpha = 2.4
mu = 10
while True:
    cascade = simulate_marked_exp_hawkes_process((p,beta), m0, alpha, mu)
    t = cascade[-1,0]
    n_obs = len(cascade)
    res = MAP(cascade,t,alpha,mu)
    p_est,beta_est = res[1]
    N = prediction((p_est,beta_est), cascade, alpha, mu, t)
    message = {'type' : 'parameters','n_obs ' : n_obs,'n_supp ' : N, 'p ' : p_est,'beta ' : beta_est}
    messageJson = dumps(message)
    messageBytes = messageJson.encode('utf-8')
    producer.send('cascade_properties',value=messageBytes)


