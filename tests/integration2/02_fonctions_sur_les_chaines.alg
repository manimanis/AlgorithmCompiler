Fonction compterVoyelles(ch: chaine): entier
Début
  cpt ← 0
  Pour i de 0 à long(ch)-1 Faire
    car ← ch[i]
    Si car = "a" Ou car = "e" Ou car = "i"
       Ou car = "o" Ou car = "u" Ou car = "y"
       Ou car = "A" Ou car = "E" Ou car = "I"
       Ou car = "O" Ou car = "U" Ou car = "Y" Alors
      cpt ← cpt + 1
    Fin Si
  Fin Pour
  Retourner cpt
Fin

TDOL
Objet : Type/Nature
i, cpt : entier
car : caractere

Fonction inverserChaine(ch: chaine): chaine
Début
  res ← ""
  Pour i de long(ch)-1 à 0 Pas -1 Faire
    res ← res + ch[i]
  Fin Pour
  Retourner res
Fin

TDOL
Objet : Type/Nature
i : entier
res : chaine

Algorithme 02_fonctions_sur_les_chaines
Début
  texte ← "Algorithmique"
  Ecrire("Texte:", texte)
  Ecrire("Longueur:", long(texte))
  Ecrire("Nombre de voyelles:", compterVoyelles(texte))
  Ecrire("Inversé:", inverserChaine(texte))
Fin

TDO
Objet : Type/Nature
texte : chaine
