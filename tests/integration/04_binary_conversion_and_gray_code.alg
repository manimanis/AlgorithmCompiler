Algorithme 04_binary_conversion_and_gray_code
Début
  Répéter
    Ecrire("Donner n ? ")
    Lire(n)
  Jusqu'à 0 <= n <= 63

  n2 ← n
  cb ← ""
  Pour i de 1 à 6 Faire
    r ← n2 mod 2
    cb ← convch(r) + cb
    n2 ← n2 div 2
  Fin Pour

  cg ← cb[0]
  Pour i de 1 à long(cb) - 1 Faire
    cg ← cg + convch(abs(valeur(cb[i]) - valeur(cb[i-1])))
  Fin Pour

  Ecrire(n, "(10) =", cb, "(2)")
  Ecrire(cb, "(2) =", cg, "(gray)")
Fin

TDO
Objet : Type/Nature
n, n2, r, i : entier
cb, cg : chaine
