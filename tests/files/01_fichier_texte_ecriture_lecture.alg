Algorithme test_fich_texte
Début
  Ouvrir("f_texte1.tmp", f, "w")
  Ecrire(f, "Première ligne")
  Fermer(f)

  Ouvrir("f_texte1.tmp", f, "r")
  Lire(f, contenu)
  Fermer(f)

  Ecrire(contenu)
Fin

TDO
Objet : Type/Nature
f : Fichier Texte
contenu : chaîne
