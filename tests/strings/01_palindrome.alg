// Test: Détection de palindrome
Fonction est_palindrome(ch: chaîne): booléen
Var d, f: entier
    pal: booléen
Début
    d ← 0
    f ← Long(ch) - 1
    pal ← Vrai
    Tant Que d < f Et pal Faire
        Si ch[d] ≠ ch[f] Alors
            pal ← Faux
        Fin Si
        d ← d + 1
        f ← f - 1
    Fin Tant Que
    Retourner pal
Fin

TDOL
Objet : Type/Nature
d, f : entier
pal : booléen

Algorithme 01_palindrome
Début
    Ecrire("RADAR :", est_palindrome("RADAR"))
    Ecrire("KAYAK :", est_palindrome("KAYAK"))
    Ecrire("ALGO :", est_palindrome("ALGO"))
    Ecrire("RESSASSER :", est_palindrome("RESSASSER"))
Fin
