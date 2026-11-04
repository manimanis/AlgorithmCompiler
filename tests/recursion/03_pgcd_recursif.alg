// Test: PGCD récursif (Euclide)
Fonction pgcd(a: entier, b: entier): entier
Début
    Si b = 0 Alors
        Retourner a
    Sinon
        Retourner pgcd(b, a mod b)
    Fin Si
Fin

Algorithme 03_pgcd_recursif
Début
    Ecrire("pgcd(54, 24) =", pgcd(54, 24))
    Ecrire("pgcd(101, 103) =", pgcd(101, 103))
    Ecrire("pgcd(48, 18) =", pgcd(48, 18))
Fin
