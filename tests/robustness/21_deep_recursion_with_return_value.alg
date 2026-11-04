Fonction fact(n: entier): entier
    Début
      Si n <= 1 Alors
        Retourner 1
      Sinon
        Retourner n * fact(n - 1)
      Fin Si
    Fin

Algorithme 21_deep_recursion_with_return_value
Début
      Ecrire(fact(10))
    Fin
