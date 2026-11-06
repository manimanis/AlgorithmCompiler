Algorithme test_alias_partage
Début
  Allouer(p1)
  p1^.x <- 100
  p1^.y <- 200

  // p2 pointe sur le meme enregistrement que p1
  p2 <- p1
  Ecrire("p2 initial:", p2^.x, p2^.y)

  // Modification via p2
  p2^.x <- 999
  Ecrire("p1 apres modif p2:", p1^.x)

  // Les pointeurs sont egaux
  Si p1 = p2 Alors
    Ecrire("p1 et p2 sont egaux")
  FinSi

  // Allocation d'un nouvel objet pour p2
  Allouer(p2)
  p2^.x <- 555
  Si p1 <> p2 Alors
    Ecrire("p1 et p2 sont differents")
    Ecrire("p1^.x =", p1^.x, "p2^.x =", p2^.x)
  FinSi
Fin

TDNT
Point2D = Enregistrement
  x, y : Entier
Fin

TDO
Objet : Type/Nature
p1, p2 : ^Point2D
