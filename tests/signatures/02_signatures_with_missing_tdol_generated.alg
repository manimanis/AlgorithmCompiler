// Test: Signatures et génération automatique de TDOL / TDO
Procédure Inverser(@t: tab, n: entier)
Début
    i ← 0
    j ← n - 1
    Tant Que i < j Faire
        aux ← t[i]
        t[i] ← t[j]
        t[j] ← aux
        i ← i + 1
        j ← j - 1
    Fin Tant Que
Fin

Fonction Somme(t: tab, n: entier): entier
Début
    s ← 0
    Pour k de 0 à n - 1 Faire
        s ← s + t[k]
    Fin Pour
    Retourner s
Fin

Algorithme TestSignaturesGenerated
Début
    t[0] ← 10
    t[1] ← 20
    t[2] ← 30
    n ← 3
    Inverser(t, n)
    res ← Somme(t, n)
    Ecrire(t[0], t[1], t[2], res)
Fin

TDNT
tab = Tableau de 10 entier
