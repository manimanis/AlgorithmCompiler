// Test: Somme des chiffres d'un entier
Fonction somme_chiffres(n: entier): entier
Var s, temp: entier
Début
    s ← 0
    temp ← n
    Tant Que temp > 0 Faire
        s ← s + (temp mod 10)
        temp ← temp div 10
    Fin Tant Que
    Retourner s
Fin

TDOL
Objet : Type/Nature
s, temp : entier

Algorithme 02_somme_chiffres
Début
    Ecrire("somme(12345) =", somme_chiffres(12345))
    Ecrire("somme(987) =", somme_chiffres(987))
    Ecrire("somme(42) =", somme_chiffres(42))
Fin
