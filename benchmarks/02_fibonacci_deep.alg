// Benchmark: Fibonacci Récursif Profond (fib(24) = 46368, ~150 000 appels)
Fonction fib(n: entier): entier
Début
    Si n ≤ 1 Alors
        Retourner n
    Sinon
        Retourner fib(n - 1) + fib(n - 2)
    Fin Si
Fin

Algorithme benchmark_fib
Début
    Ecrire("Calcul de fib(24)...")
    res ← fib(24)
    Ecrire("Résultat: fib(24) = ", res)
Fin

TDO
Objet : Type/Nature
res : entier
