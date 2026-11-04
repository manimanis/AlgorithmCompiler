Algorithme test_fich_tab_rec_imbrique
Début
  Ouvrir("f_nested_eleves.dat", f, "wb")
  e.nom ← "Amira"
  e.date_eval.j ← 25
  e.date_eval.m ← 10
  e.date_eval.a ← 2026
  e.notes[0] ← 17.5
  e.notes[1] ← 18.0
  e.notes[2] ← 16.5
  Ecrire(f, e)

  e.nom ← "Bilel"
  e.date_eval.j ← 26
  e.date_eval.m ← 10
  e.date_eval.a ← 2026
  e.notes[0] ← 14.0
  e.notes[1] ← 15.5
  e.notes[2] ← 13.0
  Ecrire(f, e)
  Fermer(f)

  // Relecture et calcul de moyenne
  Ouvrir("f_nested_eleves.dat", f, "rb")
  Tant Que Non Fin_Fichier(f) Faire
    Lire(f, res)
    somme ← 0.0
    Pour i de 0 à 2 Faire
      somme ← somme + res.notes[i]
    Fin Pour
    moy ← somme / 3.0
    Ecrire(res.nom, "(", res.date_eval.j, "/", res.date_eval.m, ") Moyenne =", moy)
  Fin Tant Que
  Fermer(f)
Fin

TDNT
DateEval = Enregistrement
  j, m, a : entier
Fin
TabNotes = Tableau de 3 Réel
Eleve = Enregistrement
  nom : chaîne
  date_eval : DateEval
  notes : TabNotes
Fin
FichEleve = Fichier de Eleve

TDO
Objet : Type/Nature
f : FichEleve
e, res : Eleve
i : entier
somme, moy : réel
