"""A faire"""


param = """{"type ": "parameters", "cid ": "tw19544", "n_obs ": 62, "n_supp ": 964.3129062676379, "p ": 0.014465944394798723, "beta ": 0.0002318378565079919, "G1 ": 445.4646147604469}"""
size = """{ 'type' : 'size', 'cid' : tw18106 , 'n_tot' : 25 , 't_end' : 3715}"""


import Data_Base

liste1 = param.replace("\"","'")
param = liste1.split("'")
size = size.split("'")

if Data_Base.type_message(param) == "parameters":
    print("Test type_message parameters OK")
else:
    print("Test type_message parameters KO")


if Data_Base.type_message(size) == "size":
    print("Test type_message size OK")
else:
    print("Test type_message size KO")

if Data_Base.id_message(param) == "tw19544":
    print("Test id_message param OK")
else:
    print("Test id_message param KO")

if Data_Base.id_message(size) == "tw18106":
    print("Test id_message size OK")
else:
    print("Test id_message size KO")

if Data_Base.p_message(param) == 0.014465944394798723:
    print("Test p_message OK")
else:
    print("Test p_message KO")

if Data_Base.beta_message(param) == 0.0002318378565079919:
    print("Test beta_message OK")
else:
    print("Test beta_message KO")

if Data_Base.G1_message(param) == 445.4646147604469:
    print("Test G1_message OK")
else:
    print("Test G1_message KO")

if Data_Base.n_partiel(param) == 62.0:
    print("Test n_partiel OK")
else:
    print("Test n_partiel KO")

if Data_Base.n_tot_message(size) == 25.0:
    print("Test n_tot_message OK")
else:
    print("Test n_tot_message KO")
