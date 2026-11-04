Fonction pgcd(a, b: entier): entier
Début
  Tant Que b != 0 Faire
    r ← a mod b
    a ← b
    b ← r
  Fin Tant Que
  Retourner a
Fin

TDOL
Objet : Type/Nature
r : entier

Fonction ppcm(a, b: entier): entier
Début
  Retourner (a * b) / pgcd(a, b)
Fin

Algorithme 01_calcul_du_pgcd_et_ppcm
Début
  x ← 48; y ← 36
  Ecrire("PGCD(", x, ",", y, ") =", pgcd(x, y))
  Ecrire("PPCM(", x, ",", y, ") =", ppcm(x, y))
Fin

TDO
Objet : Type/Nature
x, y : entier
