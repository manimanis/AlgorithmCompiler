Algorithme 01_declaration_et_affectation_chaine
Début
  x ← "Hello"
  Ecrire(x)
  y ← x[0]
  Ecrire(y)
  z ← 0
  Pour i de 0 à Long(x)-1 Faire
    y <- y + x[i]
    z ← z + (x[i] = x[i])
  Fin Pour
  Ecrire(y)
  Ecrire(z)
Fin

TDO
Objet : Type/Nature
x, y : chaîne
