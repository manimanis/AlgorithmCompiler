// Test: Nombres premiers (Crible simple)
Fonction est_premier(n: entier): booléen
Var d: entier
    premier: booléen
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

Algorithme 01_nombres_premiers
Début
    Pour i de 2 à 20 Faire
        Si est_premier(i) Alors
            Ecrire(i, fin=" ")
        Fin Si
    Fin Pour
    Ecrire("")
Fin

TDO
Objet : Type/Nature
i : entier
