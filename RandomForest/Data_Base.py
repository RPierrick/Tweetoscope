""" Data Base"""

#Date : 13/11/2020
#Auteur : Team 13 SDI (Charlotte A., Simon H. Pierrick H.)
#Objectif : Obtenir les paramètres d'un processus de Hawkes en utilisant la vraissemblance

#----------- Import -----------#

import numpy as np
from kafka import KafkaProducer,KafkaConsumer
from json import dumps
import re
import pandas as pd
from datetime import datetime
import sys

#----------- Programme --------#

consumerProperties = { "bootstrap_servers":['localhost:9092'], 
                       "auto_offset_reset":"earliest",
                       "group_id":"myOwnPrivatePythonGroup" }

consumer = KafkaConsumer(**consumerProperties)
consumer.subscribe("cascade_properties")

df = pd.DataFrame({'beta' : [],'n_star' : [], 'G1' : [], 'n_obs' : [], 'W' : []})
dico = {}

def type_message(liste):
    type_m = liste[3]
    return(type_m)

def id_message(liste):
    if type_message(liste) == 'parameters':
        id_m = liste[7]
    else:
        id_m = liste[6][3:-3]
    return(id_m)

def p_message(liste):
    if type_message(liste) == 'parameters':
        p = liste[14][2:-2]
        return(float(p))
    
def beta_message(liste):
    if type_message(liste) == 'parameters':
        beta = liste[16][2:-2]
        return(float(beta))

def G1_message(liste):
    if type_message(liste) == 'parameters':
        G1 = liste[-1][2:-1]
        return(float(G1))

def calcul_n_star(p,alpha,mu):
    return(p*mu*((alpha-1)/(alpha-2)))

def n_tot_message(liste):
    if type_message(liste) == 'size':
        ntot = liste[8][3:-2]
        return(float(ntot))

def n_partiel(liste):
    if type_message(liste) == 'parameters':
        npar = liste[10][2:-2]
        return(float(npar))

def coeff(n_tot,n_partiel,G1,n_star):
    try:
        W = ((n_tot-n_partiel)*(1-n_star))/G1
    except ZeroDivisionError:
        W = 0
    return(W)

def creation_db(chemin,taille_df):

    print("Création de la database en cours...")

    mu = 10
    alpha = 2.4


    for record in consumer:
        message = record.value.decode()
        message_2 = message.replace("\"","'")
        message = message_2.split("'")
        if type_message(message) == 'parameters':
            p,beta,G1,nobs = p_message(message),beta_message(message),G1_message(message),n_partiel(message)
            df = df.append(pd.DataFrame({'beta' : [beta],'n_star' : [p*mu*(alpha-1)/(alpha-2)], 'G1' : [G1], 'n_obs' : [nobs], 'W' : [np.nan]},index = [id_message(message)]))
        elif type_message(message) == 'size':
            if id_message(message) in df.index:
                n_tot = n_tot_message(message)
                G1,n_par,nstar = df.loc[[id_message(message)],['G1']].values,df.loc[[id_message(message)],['n_obs']].values,df.loc[[id_message(message)],['n_star']].values
                target = coeff(n_tot,n_par[0][0],G1[0][0],nstar[0][0])
                df.loc[[id_message(message)],['W']] = target
        if df['W'].count() == taille_df:
            print(df['W'].count())
            df.to_csv(chemin,index=False)
            break

    print('Fin')
    

if __name__ == '__main__':
    globals()[sys.argv[1]](sys.argv[2][0],sys.argv[2][1])
