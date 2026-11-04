// Test: Fibonacci récursif
Fonction fib(n: entier): entier
Début
    Si n ≤ 1 Alors
        Retourner n
    Sinon
        Retourner fib(n - 1) + fib(n - 2)
    Fin Si
Fin

Algorithme 01_fibonacci_recursif
Début
    Ecrire("fib(0) =", fib(0))
    Ecrire("fib(1) =", fib(1))
    Ecrire("fib(7) =", fib(7))
    Ecrire("fib(10) =", fib(10))
Fin
