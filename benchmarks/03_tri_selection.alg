// Benchmark: Tri sélection sur tableau de 400 entiers
Procédure TrierSelection(n: entier)
Début
    Pour i de 0 à n - 2 Faire
        minIdx ← i
        Pour j de i + 1 à n - 1 Faire
            Si t[j] < t[minIdx] Alors
                minIdx ← j
            Fin Si
        Fin Pour
        Si minIdx ≠ i Alors
            tmp ← t[i]
            t[i] ← t[minIdx]
            t[minIdx] ← tmp
        Fin Si
    Fin Pour
Fin

TDOL
Objet : Type/Nature
i, j, minIdx, tmp : entier

Algorithme benchmark_tri
Début
    // Remplissage en ordre décroissant (pire cas)
    Pour k de 0 à 3999 Faire
        t[k] ← 40000 - k
    Fin Pour

    TrierSelection(4000)

    Ecrire("Tri terminé. Premier: ", t[0], ", Dernier: ", t[3999])
Fin

TDNT
Tab400 = tableau de 4000 entier

TDO
Objet : Type/Nature
t : Tab400
k : entier
