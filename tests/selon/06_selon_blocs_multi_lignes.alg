Algorithme test_selon_multi_lignes
Début
  s <- 0
  Pour x De 1 a 4 Faire
    Selon x
      1 :
        s <- s + 10
        Ecrire("Cas 1:", s)
      2 .. 3 :
        s <- s + 20
        Ecrire("Cas 2 ou 3:", s)
      Sinon
        s <- s + 100
        Ecrire("Autre:", s)
    FinSelon
  Fin Pour
Fin

TDO
Objet : Type/Nature
x, s : Entier
