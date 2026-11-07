// Test: Signatures avec TDOL explicite
Procédure Saisir(@n: entier)
Début
    n ← 5
Fin

Fonction Est_Premier(n: entier): booléen
Début
    premier ← Vrai
    Pour d de 2 à n - 1 Faire
        Si n mod d = 0 Alors
            premier ← Faux
        Fin Si
    Fin Pour
    Retourner premier
Fin

TDOL
Objet : Type/Nature
d : entier
premier : booléen

Algorithme TestSignaturesExplicit
Début
    Saisir(n)
    Si Est_Premier(n) Alors
        Ecrire("Premier")
    Sinon
        Ecrire("Non premier")
    Fin Si
Fin

TDO
Objet : Type/Nature
n : entier
