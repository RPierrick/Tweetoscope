""" Random Forest """

#Date : 13/11/2020
#Auteur : Team 13 SDI (Charlotte A., Simon H. Pierrick H.)
#Objectif : Obtenir les paramètres d'un processus de Hawkes en utilisant la vraissemblance

#----------- Import -----------#

import numpy as np
from kafka import KafkaProducer,KafkaConsumer
from json import dumps
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestRegressor
from sklearn.model_selection import cross_val_score
from sklearn.metrics import mean_squared_error
from sklearn import tree
import matplotlib
import matplotlib.pyplot as plt

#----------- Programme --------#

def visualisation_test_pred(y_test,y_pred):
    for k in range(len(y_pred)):
        print(y_test[k] , '-',y_pred[k])

def visualisation_arbre_decision(regressor):
    estimator = regressor.estimators_[5]
    fig, axes = plt.subplots(nrows = 1,ncols = 1,figsize = (4,4), dpi=800)
    tree.plot_tree(estimator,
                   feature_names = X.columns, 
                   class_names=y.columns,
                   filled = True)
    fig.savefig('rf_individualtree.png')

df = pd.read_csv(chemin) #A modifier pour indiquer le chemin d'acces à "data_base.csv"

X,y = df[['beta','G1','n_star']],df[['W']]

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.1,shuffle=False)

regressor = RandomForestRegressor(n_estimators = 100,max_depth = None,bootstrap = False,
                                  min_samples_split = 2,max_leaf_nodes = len(X),random_state = 0,
                                  criterion = 'mse')

regressor.fit(X_train,y_train.values.ravel())

scores = regressor.score(X_train,y_train)
print("Le score sur les données d'entrainement est de : ", scores)

y_pred = regressor.predict(X_test)

mse = mean_squared_error(y_test,y_pred)
print("Le MSE sur des données non vues est de : ",mse)

#visualisation_test_pred(y_test.values.ravel(),y_pred)

#visualisation_arbre_decision(regressor)

