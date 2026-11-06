Procédure Enfiler (@tete : ^Element, @queue : ^Element, v : Entier)
Début
  Allouer(nouveau)
  nouveau^.val <- v
  nouveau^.suiv <- NIL
  Si queue = NIL Alors
    tete <- nouveau
    queue <- nouveau
  Sinon
    queue^.suiv <- nouveau
    queue <- nouveau
  FinSi
Fin

TDOL
Objet : Type/Nature
nouveau : ^Element

Procédure Defiler (@tete : ^Element, @queue : ^Element, @res : Entier)
Début
  Si tete <> NIL Alors
    res <- tete^.val
    tete <- tete^.suiv
    Si tete = NIL Alors
      queue <- NIL
    FinSi
  FinSi
Fin

Algorithme test_file_pointeur
Début
  tete <- NIL
  queue <- NIL

  Enfiler(tete, queue, 100)
  Enfiler(tete, queue, 200)
  Enfiler(tete, queue, 300)

  TantQue tete <> NIL Faire
    Defiler(tete, queue, item)
    Ecrire("Defile:", item)
  FinTantQue
Fin

TDNT
Element = Enregistrement
  val : Entier
  suiv : ^Element
Fin

TDO
Objet : Type/Nature
tete, queue : ^Element
item : Entier
Enfiler, Defiler : Procédure
