Procédure InitialiserGlobal()
Début
    global_cpt ← 10
Fin

Procédure IncrementerGlobal()
Début
    step_val ← 1
    global_cpt ← global_cpt + step_val
Fin

TDOL
Objet : Type/Nature
step_val : entier

Fonction ObtenirGlobalDouble() : entier
Début
    mult ← 2
    Retourner global_cpt * mult
Fin

TDOL
Objet : Type/Nature
mult : entier

Algorithme test_portee_tdo_tdol
Début
    InitialiserGlobal()
    IncrementerGlobal()
    Ecrire("global_cpt après incrément :", global_cpt)
    Ecrire("ObtenirGlobalDouble :", ObtenirGlobalDouble())
Fin

TDO
Objet : Type/Nature
global_cpt : entier
