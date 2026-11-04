Procédure Saisir(@n: entier)
Début
  Répéter
    Ecrire("Donner n ? ")
    Lire(n)
  Jusqu'à 5 <= n <= 20
Fin

Procédure Remplir(@t: tab, n:entier)
Début
  Pour i de 0 à n-1 Faire
    Lire(t[i])
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i : entier

Procédure Trier(@t: tab, n:entier)
Début
  Pour i de 0 à n-2 Faire
    Pour j de i+1 à n-1 Faire
      Si t[i] > t[j] Alors
        Echanger(t[i], t[j])
      Fin Si
    Fin Pour
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i, j : entier

Procedure Echanger(@a: entier, @b: entier)
Début
  tmp ← a
  a ← b
  b ← tmp
Fin

TDOL
Objet : Type/Nature
tmp : entier

Algorithme 05_array_sorting_with_procedures
Début
  Saisir(n)
  Remplir(t, n)
  Trier(t, n)
  Pour i de 0 à n-1 Faire
    Ecrire(t[i])
  Fin Pour
Fin

TDNT
tab = tableau de 20 entier

TDO
Objet : Type/Nature
n, s : entier
t : tab
