Algorithme test_liste_simple
Début
  tete <- NIL

  // Insertion de 30, puis 20, puis 10 au début
  Allouer(nouveau_noeud)
  nouveau_noeud^.val <- 30
  nouveau_noeud^.suiv <- tete
  tete <- nouveau_noeud

  Allouer(nouveau_noeud)
  nouveau_noeud^.val <- 20
  nouveau_noeud^.suiv <- tete
  tete <- nouveau_noeud

  Allouer(nouveau_noeud)
  nouveau_noeud^.val <- 10
  nouveau_noeud^.suiv <- tete
  tete <- nouveau_noeud

  // Parcours de la liste
  courant <- tete
  TantQue courant <> NIL Faire
    Ecrire("Element:", courant^.val)
    courant <- courant^.suiv
  FinTantQue
Fin

TDNT
Noeud = Enregistrement
  val : Entier
  suiv : ^Noeud
Fin

TDO
Objet : Type/Nature
tete, courant, nouveau_noeud : ^Noeud
