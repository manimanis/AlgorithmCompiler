Algorithme test_fich_filtre_imbrique
Début
  // 1. Initialisation fichier source
  Ouvrir("f_nested_rec.dat", f_in, "wb")
  c.id ← 1
  c.info.nom ← "Societe A"
  c.info.ville ← "Tunis"
  c.solde ← 1500.0
  Ecrire(f_in, c)

  c.id ← 2
  c.info.nom ← "Societe B"
  c.info.ville ← "Bizerte"
  c.solde ← 2300.0
  Ecrire(f_in, c)

  c.id ← 3
  c.info.nom ← "Societe C"
  c.info.ville ← "Tunis"
  c.solde ← 4100.0
  Ecrire(f_in, c)
  Fermer(f_in)

  // 2. Filtrer vers f_out pour ville = Tunis
  Ouvrir("f_nested_rec.dat", f_in, "rb")
  Ouvrir("f_clients_tunis.dat", f_out, "wb")
  nb ← 0
  Tant Que Non Fin_Fichier(f_in) Faire
    Lire(f_in, c)
    Si c.info.ville = "Tunis" Alors
      Ecrire(f_out, c)
      nb ← nb + 1
    Fin Si
  Fin Tant Que
  Fermer(f_in)
  Fermer(f_out)

  Ecrire("Nombre de clients a Tunis :", nb)

  // 3. Affichage du fichier filtré
  Ouvrir("f_clients_tunis.dat", f_out, "rb")
  Tant Que Non Fin_Fichier(f_out) Faire
    Lire(f_out, c)
    Ecrire("Client:", c.id, "-", c.info.nom, "Solde:", c.solde)
  Fin Tant Que
  Fermer(f_out)
Fin

TDNT
InfoClient = Enregistrement
  nom : chaîne
  ville : chaîne
Fin
Compte = Enregistrement
  id : entier
  info : InfoClient
  solde : réel
Fin
FichCompte = Fichier de Compte

TDO
Objet : Type/Nature
f_in, f_out : FichCompte
c : Compte
nb : entier
