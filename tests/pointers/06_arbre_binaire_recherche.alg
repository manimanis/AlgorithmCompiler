Procédure Inserer (@racine : ^NoeudA, v : Entier)
Début
  Si racine = NIL Alors
    Allouer(racine)
    racine^.cle <- v
    racine^.gauche <- NIL
    racine^.droite <- NIL
  Sinon
    Si v < racine^.cle Alors
      Inserer(racine^.gauche, v)
    Sinon
      Inserer(racine^.droite, v)
    FinSi
  FinSi
Fin

Procédure Infixe (r : ^NoeudA)
Début
  Si r <> NIL Alors
    Infixe(r^.gauche)
    Ecrire(r^.cle)
    Infixe(r^.droite)
  FinSi
Fin

Algorithme test_arbre_binaire
Début
  arbre <- NIL
  Inserer(arbre, 50)
  Inserer(arbre, 30)
  Inserer(arbre, 70)
  Inserer(arbre, 20)
  Inserer(arbre, 40)
  Inserer(arbre, 60)
  Inserer(arbre, 80)

  Ecrire("Parcours infixe:")
  Infixe(arbre)
Fin

TDNT
NoeudA = Enregistrement
  cle : Entier
  gauche : ^NoeudA
  droite : ^NoeudA
Fin

TDO
Objet : Type/Nature
arbre : ^NoeudA
Inserer, Infixe : Procédure
