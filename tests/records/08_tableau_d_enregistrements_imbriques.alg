Algorithme test_tab_rec_imbrique
Début
  V[0].ref ← "P01"
  V[0].dim.largeur ← 10.0
  V[0].dim.hauteur ← 20.0
  V[1].ref ← "P02"
  V[1].dim.largeur ← 15.0
  V[1].dim.hauteur ← 30.0
  V[2].ref ← "P03"
  V[2].dim.largeur ← 8.0
  V[2].dim.hauteur ← 12.0

  Pour i de 0 à 2 Faire
    surf ← V[i].dim.largeur * V[i].dim.hauteur
    Ecrire(V[i].ref, ": Surface =", surf)
  Fin Pour
Fin

TDNT
Dimension = Enregistrement
  largeur, hauteur : réel
Fin
Panneau = Enregistrement
  ref : chaîne
  dim : Dimension
Fin
TabPanneau = Tableau de 3 Panneau

TDO
Objet : Type/Nature
V : TabPanneau
i : entier
surf : réel
