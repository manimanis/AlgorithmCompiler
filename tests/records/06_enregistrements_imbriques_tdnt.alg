Algorithme test_rec_imbrique_tdnt
Début
  e.matricule ← 1024
  e.nom ← "Salah"
  e.date_naiss.jour ← 12
  e.date_naiss.mois ← 5
  e.date_naiss.annee ← 2004
  Ecrire("Etudiant:", e.matricule, e.nom)
  Ecrire("Ne le:", e.date_naiss.jour, "/", e.date_naiss.mois, "/", e.date_naiss.annee)
Fin

TDNT
DateNaiss = Enregistrement
  jour, mois, annee : entier
Fin
Etudiant = Enregistrement
  matricule : entier
  nom : chaîne
  date_naiss : DateNaiss
Fin

TDO
Objet : Type/Nature
e : Etudiant
