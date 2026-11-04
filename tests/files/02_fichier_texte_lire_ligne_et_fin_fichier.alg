Algorithme test_fich_lignes
Début
  Ouvrir("f_texte2.tmp", f, "w")
  Ecrire_nl(f, "Ligne 1")
  Ecrire_nl(f, "Ligne 2")
  Ecrire_nl(f, "Ligne 3")
  Fermer(f)

  Ouvrir("f_texte2.tmp", f, "r")
  Tant Que Non Fin_fichier(f) Faire
    Lire_ligne(f, ch)
    Ecrire("Lue:", ch)
  Fin Tant Que
  Fermer(f)
Fin

TDO
Objet : Type/Nature
f : Fichier Texte
ch : chaîne
