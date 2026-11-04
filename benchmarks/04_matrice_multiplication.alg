// Benchmark: Multiplication de matrices 25x25 (15 625 opérations x 5 passes)
Algorithme benchmark_matrice
Début
    // Initialisation
    Pour i de 0 à 24 Faire
        Pour j de 0 à 24 Faire
            A[i, j] ← (i + j) mod 10
            B[i, j] ← (i * j + 1) mod 10
            C[i, j] ← 0
        Fin Pour
    Fin Pour

    // Multiplication C = A * B
    Pour i de 0 à 24 Faire
        Pour j de 0 à 24 Faire
            somme ← 0
            Pour k de 0 à 24 Faire
                somme ← somme + A[i, k] * B[k, j]
            Fin Pour
            C[i, j] ← somme
        Fin Pour
    Fin Pour

    Ecrire("C[0, 0] = ", C[0, 0], ", C[12, 12] = ", C[12, 12], ", C[24, 24] = ", C[24, 24])
Fin

TDNT
Mat25 = tableau de 25, 25 entier

TDO
Objet : Type/Nature
A, B, C : Mat25
i, j, k, somme : entier
