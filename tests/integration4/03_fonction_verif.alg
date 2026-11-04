Fonction verif(ch: chaîne):booléen
Début
  test ← Vrai
  i ← 0
  Tant Que test et i < Long(ch) Faire
    test ← "A" ≤ ch[i] ≤ "Z"
    i ← i + 1
  Fin Tant Que
  Retourner test
Fin

TDOL
Objet : Type/Nature
i : entier
test : booléen

Algorithme 03_fonction_verif
Début
  x ← verif("Hello")
  y ← verif("HELLO")
  Ecrire(x)
  Ecrire(y)
Fin

TDO
Objet : Type/Nature
x,y : booléen
