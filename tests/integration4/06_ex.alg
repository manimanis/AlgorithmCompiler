Procédure Filtrer(TB: Tab1, N: Entier, @TV: Tab1, @M: Entier)
Début
  M ← 0
  Pour i de 0 à N-1 Faire
    Si TB[i] > 0 Alors
      TV[M] ← TB[i]
      M ← M + 1
    Fin Si
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i : Entier

Procédure Trier(@TV: Tab1, M: entier)
Début
  Pour i de 0 à M-2 Faire
    min ← i
    Pour j de i+1 à M-1 Faire
      Si TV[j] < TV[min] Alors
        min ← j
      Fin Si
    Fin Pour
    Si min ≠ i Alors
      temp ← TV[i]
      TV[i] ← TV[min]
      TV[min] ← temp
    Fin Si
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i, j, min, temp : entier

Fonction Mediane(TV: Tab1, M: entier):Réel
Début
  c ← M div 2
  Si M mod 2 = 0 Alors
    med ← (TV[c] + TV[c - 1]) / 2
  Sinon
    med ← TV[c]
  Fin Si
  Retourner med
Fin

TDOL
Objet : Type/Nature
c : Entier
med : réel

Procédure Saisir(@n: entier)
Début
  Répéter
    Ecrire("Entrez un entier : ");
    Lire(n);
  Jusqu'à n > 0
Fin

Procédure Remplir(@t: Tab1, n: entier)
Début
  Pour i de 0 à n-1 Faire
    Ecrire("t[", i, "] = ");
    Lire(t[i])
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i : entier

Algorithme Intégration
Début
  Saisir(N)
  Remplir(TB, N)
  Filtrer(TB, N, TV, M)
  Trier(TV, M)
  med ← Mediane(TV, M)
  Ecrire("La production médiane :", med)
Fin

TDNT
Tab1 = Tableau de 31 réel

TDO
Objet : Type/Nature
N, M : Entier
med : réel
TB, TV : Tab1
