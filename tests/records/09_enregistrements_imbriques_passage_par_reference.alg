Procédure Decaler_Date (@d : DateSimple, delta_j : Entier)
Début
  d.jour ← d.jour + delta_j
Fin

Procédure MettreAJour_Membre (@m : Membre, nouveau_statut : Chaîne)
Début
  m.statut ← nouveau_statut
  Decaler_Date(m.adhesion, 30)
Fin

Algorithme test_ref_rec_imbrique
Début
  m.nom ← "Kamel"
  m.statut ← "Actif"
  m.adhesion.jour ← 1
  m.adhesion.mois ← 9
  m.adhesion.annee ← 2026

  Ecrire("Initial:", m.nom, m.statut, m.adhesion.jour)
  MettreAJour_Membre(m, "Renouvele")
  Ecrire("Apres:", m.nom, m.statut, m.adhesion.jour)
Fin

TDNT
DateSimple = Enregistrement
  jour, mois, annee : entier
Fin
Membre = Enregistrement
  nom : chaîne
  statut : chaîne
  adhesion : DateSimple
Fin

TDO
Objet : Type/Nature
m : Membre
Decaler_Date, MettreAJour_Membre : Procédure
