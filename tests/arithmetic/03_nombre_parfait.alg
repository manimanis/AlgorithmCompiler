// Test: Nombres parfaits (somme des diviseurs stricts = n)
Fonction est_parfait(n: entier): booléen
Var s, d: entier
Début
    s ← 0
    Pour d de 1 à n - 1 Faire
        Si n mod d = 0 Alors
            s ← s + d
        Fin Si
    Fin Pour
    Retourner s = n
Fin

TDOL
Objet : Type/Nature
s, d : entier

Algorithme 03_nombre_parfait
Début
    Ecrire("6 :", est_parfait(6))
    Ecrire("10 :", est_parfait(10))
    Ecrire("28 :", est_parfait(28))
    Ecrire("496 :", est_parfait(496))
Fin
