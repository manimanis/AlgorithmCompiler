Algorithme test_tab_rec
Début
  promo[0].nom ← "Sami"
  promo[0].age ← 18
  promo[1].nom ← "Mouna"
  promo[1].age ← 19
  Pour i De 0 A 1 Faire
    Ecrire(promo[i].nom, promo[i].age)
  Fin Pour
Fin

TDNT
Etudiant = Enregistrement
  nom : chaîne
  age : entier
Fin

TDO
Objet : Type/Nature
promo : Tableau de 2 Etudiant
i : entier
