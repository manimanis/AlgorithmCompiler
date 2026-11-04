Algorithme test_fich_data_rec
Début
  Ouvrir("f_rec.tmp", f, "wb")
  p.nom ← "Foulen"
  p.score ← 19.5
  Ecrire(f, p)
  p.nom ← "Falten"
  p.score ← 15.0
  Ecrire(f, p)
  Fermer(f)

  Ouvrir("f_rec.tmp", f, "rb")
  Tant Que Non Fin_fichier(f) Faire
    Lire(f, res)
    Ecrire(res.nom, ":", res.score)
  Fin Tant Que
  Fermer(f)
Fin

TDNT
Eleve = Enregistrement
  nom : chaîne
  score : réel
Fin

TDO
Objet : Type/Nature
f : Fichier de Eleve
p, res : Eleve
