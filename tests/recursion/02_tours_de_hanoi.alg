// Test: Tours de Hanoï
Procédure hanoi(n: entier, dep: chaîne, dest: chaîne, aux: chaîne)
Début
    Si n = 1 Alors
        Ecrire("Deplacer disque 1 de", dep, "a", dest)
    Sinon
        hanoi(n - 1, dep, aux, dest)
        Ecrire("Deplacer disque", n, "de", dep, "a", dest)
        hanoi(n - 1, aux, dest, dep)
    Fin Si
Fin

Algorithme 02_tours_de_hanoi
Début
    hanoi(3, "A", "C", "B")
Fin
