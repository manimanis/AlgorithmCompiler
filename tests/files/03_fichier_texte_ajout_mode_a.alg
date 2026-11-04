Algorithme test_fich_ajout
Début
  Ouvrir("f_texte3.tmp", f, "w")
  Ecrire_nl(f, "Init")
  Fermer(f)

  Ouvrir("f_texte3.tmp", f, "a")
  Ecrire_nl(f, "Suite")
  Fermer(f)

  Ouvrir("f_texte3.tmp", f, "r")
  Tant Que Non Fin_fichier(f) Faire
    Lire_ligne(f, ligne)
    Ecrire(ligne)
  Fin Tant Que
  Fermer(f)
Fin

TDO
Objet : Type/Nature
f : Fichier Texte
ligne : chaîne
