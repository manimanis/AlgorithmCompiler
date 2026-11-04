Algorithme test_rec_multi_niveaux
Début
  d.id ← 55
  d.titre ← "Projet Alpha"
  d.responsable.nom ← "Ben Salah"
  d.responsable.contact.email ← "salah@domaine.tn"
  d.responsable.contact.tel ← 98123456
  Ecrire("Dossier", d.id, ":", d.titre)
  Ecrire("Responsable:", d.responsable.nom)
  Ecrire("Email:", d.responsable.contact.email)
  Ecrire("Tel:", d.responsable.contact.tel)
Fin

TDNT
Coordonnees = Enregistrement
  email : chaîne
  tel : entier
Fin
Agent = Enregistrement
  nom : chaîne
  contact : Coordonnees
Fin
Dossier = Enregistrement
  id : entier
  titre : chaîne
  responsable : Agent
Fin

TDO
Objet : Type/Nature
d : Dossier
