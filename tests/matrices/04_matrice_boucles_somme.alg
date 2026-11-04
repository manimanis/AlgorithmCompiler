Algorithme test_matrice_boucles
Début
  somme ← 0
  Pour i De 0 A 1 Faire
    Pour j De 0 A 2 Faire
      M[i, j] ← i + j
      somme ← somme + M[i, j]
    Fin Pour
  Fin Pour
  Ecrire("Somme =", somme)
Fin

TDO
Objet : Type/Nature
M : Tableau de 2 lignes * 3 colonnes entier
i, j, somme : entier
