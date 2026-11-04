Algorithme test_fich_data_int
Début
  Ouvrir("f_data1.tmp", f, "wb")
  x ← 42
  Ecrire(f, x)
  x ← 84
  Ecrire(f, x)
  Fermer(f)

  Ouvrir("f_data1.tmp", f, "rb")
  Tant Que Non Fin_fichier(f) Faire
    Lire(f, val)
    Ecrire("Val:", val)
  Fin Tant Que
  Fermer(f)
Fin

TDO
Objet : Type/Nature
f : Fichier de entier
x, val : entier
