Procédure AjouterVoisin (@sommet : ^SommetGraphe, dest_id : Entier)
Début
  Allouer(nouveau_voisin)
  nouveau_voisin^.sommet_dest <- dest_id
  nouveau_voisin^.suiv <- sommet^.voisins
  sommet^.voisins <- nouveau_voisin
Fin

TDOL
Objet : Type/Nature
nouveau_voisin : ^Arete

Algorithme test_graphe
Début
  // Creation d'un sommet 1
  Allouer(s1)
  s1^.id <- 1
  s1^.voisins <- NIL

  // Ajout de voisins 2 et 3
  AjouterVoisin(s1, 2)
  AjouterVoisin(s1, 3)

  Ecrire("Sommet:", s1^.id)
  v <- s1^.voisins
  TantQue v <> NIL Faire
    Ecrire("Voisin:", v^.sommet_dest)
    v <- v^.suiv
  FinTantQue
Fin

TDNT
Arete = Enregistrement
  sommet_dest : Entier
  suiv : ^Arete
Fin
SommetGraphe = Enregistrement
  id : Entier
  voisins : ^Arete
Fin

TDO
Objet : Type/Nature
s1 : ^SommetGraphe
v : ^Arete
AjouterVoisin : Procédure
