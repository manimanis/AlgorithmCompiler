const fs = require('fs');

fs.mkdirSync('tests/matrices', { recursive: true });
fs.mkdirSync('tests/records', { recursive: true });
fs.mkdirSync('tests/files', { recursive: true });

// 01_matrice_declaration_et_acces.out
fs.writeFileSync('tests/matrices/01_matrice_declaration_et_acces.out', '10 20\r\n30 40\r\n');

// 02_matrice_tdnt
fs.writeFileSync('tests/matrices/02_matrice_tdnt.alg', 
`Algorithme test_matrice_tdnt
Début
  M[0, 0] ← 1.5
  M[1, 1] ← 2.5
  M[2, 2] ← 3.5
  Ecrire(M[0, 0], M[1, 1], M[2, 2])
Fin

TDNT
Mat3 = Tableau de 3 lignes * 3 colonnes réel

TDO
Objet : Type/Nature
M : Mat3
`);
fs.writeFileSync('tests/matrices/02_matrice_tdnt.out', '1.5 2.5 3.5\r\n');

// 03_matrice_double_crochet
fs.writeFileSync('tests/matrices/03_matrice_double_crochet.alg', 
`Algorithme test_matrice_double_crochet
Début
  M[0][0] ← 5
  M[0][1] ← 10
  M[1][0] ← 15
  M[1][1] ← 20
  Ecrire(M[0][0], M[0][1])
  Ecrire(M[1][0], M[1][1])
Fin

TDO
Objet : Type/Nature
M : Tableau de 2 lignes * 2 colonnes entier
`);
fs.writeFileSync('tests/matrices/03_matrice_double_crochet.out', '5 10\r\n15 20\r\n');

// 04_matrice_boucles_somme
fs.writeFileSync('tests/matrices/04_matrice_boucles_somme.alg', 
`Algorithme test_matrice_boucles
Début
  somme ← 0
  Pour i De 0 A 1 Faire
    Pour j De 0 A 2 Faire
      M[i, j] ← i + j
      somme ← somme + M[i, j]
    Fin Pour
  Fin Pour
  Ecrire("Somme =", somme)
Fin

TDO
Objet : Type/Nature
M : Tableau de 2 lignes * 3 colonnes entier
i, j, somme : entier
`);
fs.writeFileSync('tests/matrices/04_matrice_boucles_somme.out', 'Somme = 6\r\n');

// 05_matrice_saisie_lire
fs.writeFileSync('tests/matrices/05_matrice_saisie_lire.alg', 
`Algorithme test_matrice_saisie
Début
  Pour i De 0 A 1 Faire
    Pour j De 0 A 1 Faire
      Lire(M[i, j])
    Fin Pour
  Fin Pour
  Ecrire(M[0, 0], M[0, 1])
  Ecrire(M[1, 0], M[1, 1])
Fin

TDO
Objet : Type/Nature
M : Tableau de 2 lignes * 2 colonnes entier
i, j : entier
`);
fs.writeFileSync('tests/matrices/05_matrice_saisie_lire.in', '100\r\n200\r\n300\r\n400\r\n');
fs.writeFileSync('tests/matrices/05_matrice_saisie_lire.out', '100 200\r\n300 400\r\n');

// Records tests
// 01_enregistrement_tdnt
fs.writeFileSync('tests/records/01_enregistrement_tdnt.alg', 
`Algorithme test_rec_tdnt
Début
  p.nom ← "Ben Salah"
  p.age ← 25
  Ecrire(p.nom, p.age)
Fin

TDNT
Personne = Enregistrement
  nom : chaîne
  age : entier
Fin

TDO
Objet : Type/Nature
p : Personne
`);
fs.writeFileSync('tests/records/01_enregistrement_tdnt.out', 'Ben Salah 25\r\n');

// 02_enregistrement_tdo_direct
fs.writeFileSync('tests/records/02_enregistrement_tdo_direct.alg', 
`Algorithme test_rec_tdo
Début
  art.code ← 101
  art.prix ← 45.5
  Ecrire(art.code, art.prix)
Fin

TDO
Objet : Type/Nature
art : Enregistrement
  code : entier
  prix : réel
Fin
`);
fs.writeFileSync('tests/records/02_enregistrement_tdo_direct.out', '101 45.5\r\n');

// 03_enregistrement_copie_globale
fs.writeFileSync('tests/records/03_enregistrement_copie_globale.alg', 
`Algorithme test_rec_copie
Début
  p1.nom ← "Ali"
  p1.age ← 20
  p2 ← p1
  p1.nom ← "Salah"
  Ecrire(p2.nom, p2.age)
  Ecrire(p1.nom, p1.age)
Fin

TDNT
Personne = Enregistrement
  nom : chaîne
  age : entier
Fin

TDO
Objet : Type/Nature
p1, p2 : Personne
`);
fs.writeFileSync('tests/records/03_enregistrement_copie_globale.out', 'Ali 20\r\nSalah 20\r\n');

// 04_tableau_d_enregistrements
fs.writeFileSync('tests/records/04_tableau_d_enregistrements.alg', 
`Algorithme test_tab_rec
Début
  promo[0].nom ← "Sami"
  promo[0].age ← 18
  promo[1].nom ← "Mouna"
  promo[1].age ← 19
  Pour i De 0 A 1 Faire
    Ecrire(promo[i].nom, promo[i].age)
  Fin Pour
Fin

TDNT
Etudiant = Enregistrement
  nom : chaîne
  age : entier
Fin

TDO
Objet : Type/Nature
promo : Tableau de 2 Etudiant
i : entier
`);
fs.writeFileSync('tests/records/04_tableau_d_enregistrements.out', 'Sami 18\r\nMouna 19\r\n');

// 05_enregistrement_lire
fs.writeFileSync('tests/records/05_enregistrement_lire.alg', 
`Algorithme test_rec_lire
Début
  Lire(p.nom)
  Lire(p.age)
  Ecrire("Nom:", p.nom, "Age:", p.age)
Fin

TDNT
Personne = Enregistrement
  nom : chaîne
  age : entier
Fin

TDO
Objet : Type/Nature
p : Personne
`);
fs.writeFileSync('tests/records/05_enregistrement_lire.in', 'Ahmed\r\n22\r\n');
fs.writeFileSync('tests/records/05_enregistrement_lire.out', 'Nom: Ahmed Age: 22\r\n');

// Files tests
// 01_fichier_texte_ecriture_lecture
fs.writeFileSync('tests/files/01_fichier_texte_ecriture_lecture.alg', 
`Algorithme test_fich_texte
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
`);
fs.writeFileSync('tests/files/01_fichier_texte_ecriture_lecture.out', 'Première ligne\r\n');

// 02_fichier_texte_lire_ligne_et_fin_fichier
fs.writeFileSync('tests/files/02_fichier_texte_lire_ligne_et_fin_fichier.alg', 
`Algorithme test_fich_lignes
Début
  Ouvrir("f_texte2.tmp", f, "w")
  Ecrire_nl(f, "Ligne 1")
  Ecrire_nl(f, "Ligne 2")
  Ecrire_nl(f, "Ligne 3")
  Fermer(f)

  Ouvrir("f_texte2.tmp", f, "r")
  TantQue Non Fin_fichier(f) Faire
    Lire_ligne(f, ch)
    Ecrire("Lue:", ch)
  Fin TantQue
  Fermer(f)
Fin

TDO
Objet : Type/Nature
f : Fichier Texte
ch : chaîne
`);
fs.writeFileSync('tests/files/02_fichier_texte_lire_ligne_et_fin_fichier.out', 'Lue: Ligne 1\r\nLue: Ligne 2\r\nLue: Ligne 3\r\n');

// 03_fichier_texte_ajout_mode_a
fs.writeFileSync('tests/files/03_fichier_texte_ajout_mode_a.alg', 
`Algorithme test_fich_ajout
Début
  Ouvrir("f_texte3.tmp", f, "w")
  Ecrire_nl(f, "Init")
  Fermer(f)

  Ouvrir("f_texte3.tmp", f, "a")
  Ecrire_nl(f, "Suite")
  Fermer(f)

  Ouvrir("f_texte3.tmp", f, "r")
  TantQue Non Fin_fichier(f) Faire
    Lire_ligne(f, ligne)
    Ecrire(ligne)
  Fin TantQue
  Fermer(f)
Fin

TDO
Objet : Type/Nature
f : Fichier Texte
ligne : chaîne
`);
fs.writeFileSync('tests/files/03_fichier_texte_ajout_mode_a.out', 'Init\r\nSuite\r\n');

// 04_fichier_donnees_entiers
fs.writeFileSync('tests/files/04_fichier_donnees_entiers.alg', 
`Algorithme test_fich_data_int
Début
  Ouvrir("f_data1.tmp", f, "wb")
  x ← 42
  Ecrire(f, x)
  x ← 84
  Ecrire(f, x)
  Fermer(f)

  Ouvrir("f_data1.tmp", f, "rb")
  TantQue Non Fin_fichier(f) Faire
    Lire(f, val)
    Ecrire("Val:", val)
  Fin TantQue
  Fermer(f)
Fin

TDO
Objet : Type/Nature
f : Fichier de entier
x, val : entier
`);
fs.writeFileSync('tests/files/04_fichier_donnees_entiers.out', 'Val: 42\r\nVal: 84\r\n');

// 05_fichier_donnees_enregistrements
fs.writeFileSync('tests/files/05_fichier_donnees_enregistrements.alg', 
`Algorithme test_fich_data_rec
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
  TantQue Non Fin_fichier(f) Faire
    Lire(f, res)
    Ecrire(res.nom, ":", res.score)
  Fin TantQue
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
`);
fs.writeFileSync('tests/files/05_fichier_donnees_enregistrements.out', 'Foulen : 19.5\r\nFalten : 15\r\n');

console.log('All tests created successfully.');
