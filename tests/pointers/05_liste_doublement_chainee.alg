Algorithme test_liste_double
Début
  tete <- NIL
  Allouer(n1)
  n1^.val <- 10
  n1^.prec <- NIL
  n1^.suiv <- NIL
  tete <- n1

  Allouer(n2)
  n2^.val <- 20
  n2^.prec <- n1
  n2^.suiv <- NIL
  n1^.suiv <- n2

  Allouer(n3)
  n3^.val <- 30
  n3^.prec <- n2
  n3^.suiv <- NIL
  n2^.suiv <- n3

  // Parcours avant
  Ecrire("--- Avant ---")
  courant <- tete
  TantQue courant <> NIL Faire
    Ecrire(courant^.val)
    Si courant^.suiv = NIL Alors
      dernier <- courant
    FinSi
    courant <- courant^.suiv
  FinTantQue

  // Parcours arrière
  Ecrire("--- Arriere ---")
  courant <- dernier
  TantQue courant <> NIL Faire
    Ecrire(courant^.val)
    courant <- courant^.prec
  FinTantQue
Fin

TDNT
NoeudD = Enregistrement
  val : Entier
  prec : ^NoeudD
  suiv : ^NoeudD
Fin

TDO
Objet : Type/Nature
tete, dernier, courant, n1, n2, n3 : ^NoeudD
