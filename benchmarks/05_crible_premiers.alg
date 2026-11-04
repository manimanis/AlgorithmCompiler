// Benchmark: Recherche de nombres premiers (crible jusqu'à 10000)
Fonction est_premier(n: entier): booléen
Début
    Si n < 2 Alors
        Retourner Faux
    Fin Si
    premier ← Vrai
    d ← 2
    Tant Que d * d ≤ n Et premier Faire
        Si n mod d = 0 Alors
            premier ← Faux
        Fin Si
        d ← d + 1
    Fin Tant Que
    Retourner premier
Fin

TDOL
Objet : Type/Nature
d : entier
premier : booléen

Algorithme benchmark_crible
Début
    nbPremiers ← 0
    Pour x de 2 à 100000 Faire
        Si est_premier(x) Alors
            nbPremiers ← nbPremiers + 1
        Fin Si
    Fin Pour
    Ecrire("Nombre de premiers ≤ 100000: ", nbPremiers)
Fin

TDO
Objet : Type/Nature
x, nbPremiers : entier
