// Benchmark: Recherche de nombres premiers (crible jusqu'à 1000000)
// @max-iter: 200000000
// @timeout: 1200000
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
    Pour x de 2 à 1000000 Faire
        Si est_premier(x) Alors
            nbPremiers ← nbPremiers + 1
        Fin Si
    Fin Pour
    Ecrire("Nombre de premiers ≤ 1000000: ", nbPremiers)
Fin

TDO
Objet : Type/Nature
x, nbPremiers : entier
