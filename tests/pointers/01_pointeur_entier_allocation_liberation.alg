Algorithme test_pointeur_entier
Début
  p <- NIL
  Si p = NIL Alors
    Ecrire("Initialement NIL")
  FinSi

  Allouer(p)
  p^ <- 42
  Ecrire("Valeur pointee:", p^)

  p^ <- p^ + 8
  Ecrire("Nouvelle valeur:", p^)

  Liberer(p)
  Si p = NIL Alors
    Ecrire("Apres liberation NIL")
  FinSi
Fin

TDO
Objet : Type/Nature
p : ^Entier
