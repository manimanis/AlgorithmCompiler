Procédure Empiler (@sommet : ^Cellule, x : Entier)
Début
  Allouer(nouveau)
  nouveau^.valeur <- x
  nouveau^.prec <- sommet
  sommet <- nouveau
Fin

TDOL
Objet : Type/Nature
nouveau : ^Cellule

Procédure Depiler (@sommet : ^Cellule, @x : Entier)
Début
  Si sommet <> NIL Alors
    x <- sommet^.valeur
    sommet <- sommet^.prec
  FinSi
Fin

Algorithme test_pile_pointeur
Début
  pile <- NIL
  Empiler(pile, 5)
  Empiler(pile, 15)
  Empiler(pile, 25)

  TantQue pile <> NIL Faire
    Depiler(pile, val)
    Ecrire("Depile:", val)
  FinTantQue
Fin

TDNT
Cellule = Enregistrement
  valeur : Entier
  prec : ^Cellule
Fin

TDO
Objet : Type/Nature
pile : ^Cellule
val : Entier
Empiler, Depiler : Procédure
