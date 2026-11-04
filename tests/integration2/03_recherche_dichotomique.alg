Fonction rechercher(t: tab, val: entier): entier
Début
  d ← 0; f ← 9
  Tant Que d <= f Faire
    milieu ← (d + f) div 2
    Si t[milieu] = val Alors
      Retourner milieu
    Sinon Si t[milieu] < val Alors
      d ← milieu + 1
    Sinon
      f ← milieu - 1
    Fin Si
  Fin Tant Que
  Retourner -1
Fin

TDOL
Objet : Type/Nature
d, f, milieu : entier

Algorithme 03_recherche_dichotomique
Début
  Pour i de 0 à 9 Faire
    tab[i] ← i * 3
  Fin Pour

  pos ← rechercher(tab, 15)
  Si pos >= 0 Alors
    Ecrire("Valeur 15 trouvée à l'indice", pos)
  Sinon
    Ecrire("Valeur 15 non trouvée")
  Fin Si

  pos ← rechercher(tab, 24)
  Si pos >= 0 Alors
    Ecrire("Valeur 24 trouvée à l'indice", pos)
  Sinon
    Ecrire("Valeur 24 non trouvée")
  Fin Si

  pos ← rechercher(tab, 25)
  Si pos >= 0 Alors
    Ecrire("Valeur 25 trouvée à l'indice", pos)
  Sinon
    Ecrire("Valeur 25 non trouvée")
  Fin Si
Fin

TDNT
tab = tableau de 10 entier

TDO
Objet : Type/Nature
tab : tab
i, pos : entier
