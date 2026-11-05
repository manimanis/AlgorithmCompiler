Procédure Calculer (a : entier, @res : entier)
Début
    fact ← 2
    res ← a * fact
Fin

TDOL
fact : entier

Fonction Doubler (n : entier) : entier
Début
    mult ← 2
    Retourner n * mult
Fin

TDOL
mult : entier

Algorithme test_plusieurs_tdol
Début
    Calculer(5, r)
    Ecrire("Calculer(5) =", r)
    Ecrire("Doubler(10) =", Doubler(10))
Fin

TDO
Objet : Type/Nature
r : entier
