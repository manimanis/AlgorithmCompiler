Procédure tester(v : Entier)
Début
  Selon v
    1, 2, 3 : Ecrire("traitement1")
    4 .. 5 : Ecrire("traitement2")
    6, 7, 8..9 : Ecrire("traitement3")
    Sinon
      Ecrire("traitement4")
  Fin Selon
Fin

Algorithme test_selon_complet
Début
  tester(2)
  tester(4)
  tester(7)
  tester(9)
  tester(10)
Fin
