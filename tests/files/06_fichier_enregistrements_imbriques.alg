Algorithme test_fich_rec_imbrique
Début
  Ouvrir("f_nested_rec.dat", f, "wb")
  c1.code ← 101
  c1.nom ← "Moncef"
  c1.adr.ville ← "Tunis"
  c1.adr.cp ← 1002
  Ecrire(f, c1)

  c1.code ← 102
  c1.nom ← "Fatma"
  c1.adr.ville ← "Nabeul"
  c1.adr.cp ← 8000
  Ecrire(f, c1)

  c1.code ← 103
  c1.nom ← "Yassine"
  c1.adr.ville ← "Sfax"
  c1.adr.cp ← 3000
  Ecrire(f, c1)
  Fermer(f)

  // Relecture
  Ouvrir("f_nested_rec.dat", f, "rb")
  Tant Que Non Fin_Fichier(f) Faire
    Lire(f, lu)
    Ecrire(lu.code, lu.nom, "->", lu.adr.ville, lu.adr.cp)
  Fin Tant Que
  Fermer(f)
Fin

TDNT
Adresse = Enregistrement
  ville : chaîne
  cp : entier
Fin
Client = Enregistrement
  code : entier
  nom : chaîne
  adr : Adresse
Fin
FichClient = Fichier de Client

TDO
Objet : Type/Nature
f : FichClient
c1, lu : Client
