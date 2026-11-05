Procédure ModifierTableau(@tab : Vecteur)
Début
    Pour i de 0 à 4 Faire
        tab[i] ← tab[i] * 2
    Fin Pour
Fin

TDOL
Objet : Type/Nature
i : entier

Algorithme test_array_fastpath
Début
    Pour k de 0 à 4 Faire
        v[k] ← k + 1
    Fin Pour

    ModifierTableau(v)

    Pour j de 0 à 4 Faire
        Ecrire("v[", j, "] =", v[j])
    Fin Pour
Fin

TDNT
Vecteur = Tableau de 5 entier

TDO
Objet : Type/Nature
v : Vecteur
k, j : entier
ModifierTableau : Procédure
