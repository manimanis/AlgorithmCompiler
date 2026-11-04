Procédure Saisie (@c : Caractère, @n : Entier)
Début
    Ecrire("Donner un caractère : ")
    Lire(c)
    Répéter
        Ecrire("Donner une limite strictement positive (n >= 1) : ")
        Lire(n)
    Jusqu'à (n >= 1)
Fin

Fonction inconnue (c : Caractère, n : Entier) : Chaine
Début
    Si n = 1 Alors
        Retourner c + c
    Sinon
        Retourner CHR(ORD(c) + n - 1) + inconnue(c, n - 1) + CHR(ORD(c) + n - 1)
    Fin Si
Fin

Procédure Afficher_Suite (c : Caractère, n : Entier)
Début
    Pour i de 1 à n Faire
        Ecrire_NL(inconnue(c, i))
    Fin Pour
Fin

TDOL
Objet : Type/Nature
i : Entier
inconnue : Fonction

Algorithme Exercice1
Début
    Saisie(c, n)
    Afficher_Suite(c, n)
Fin

TDO
Objet : Type/Nature
c : Caractère
n : Entier
Saisie : Procédure
Afficher_Suite : Procédure
