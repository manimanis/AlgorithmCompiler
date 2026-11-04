// Benchmark: Boucle arithmétique intensive (500 000 itérations)
Algorithme benchmark_loop
Début
    somme ← 0
    Pour i de 1 à 500000 Faire
        somme ← (somme + i) mod 1000000
    Fin Pour
    Ecrire("Somme modulo: ", somme)
Fin

TDO
Objet : Type/Nature
i : entier
somme : entier
