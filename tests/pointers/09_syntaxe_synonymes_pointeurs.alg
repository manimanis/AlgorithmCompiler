Algorithme test_synonymes_pointeurs
Début
  p1 <- NIL
  Allouer(p1)
  p1^ <- 123
  Ecrire("p1:", p1^)

  Allouer(noeud)
  noeud^.val <- 456
  noeud^.suiv <- NIL
  Ecrire("noeud val:", noeud^.val)
  Liberer(p1)
  Liberer(noeud)
Fin

TDNT
PNoeud = Pointeur sur Cellule
Cellule = Enregistrement
  val : Entier
  suiv : Pointeur sur Cellule
Fin

TDO
Objet : Type/Nature
p1 : Pointeur sur Entier
noeud : PNoeud
