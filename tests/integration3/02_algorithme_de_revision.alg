Procédure Saisir(@n: entier)
Début
  Répéter
    Ecrire("Entrez un entier : ");
    Lire(n);
  Jusqu'à 4 ≤ n ≤ 10
Fin

Procédure Remplir(@indices: tab, n: entier)
Début
  Pour i de 0 à n-1 Faire
    Ecrire("Indice n°", i)
    indices[i] ← SaisieIndice(indices, i)
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i : entier

Fonction SaisieIndice(indices: tab, n: entier):chaîne
Début
  indice ← ""
  Répéter
    Ecrire("Nombre de valeurs ? ")
    Lire(nv)
  Jusqu'à nv ≥ 1

  Pour i de 1 à nv Faire
    Répéter
      Ecrire("Code n°", i, " ? ")
      Lire(code)
    Jusqu'à (33 ≤ code ≤ 127) et (Premier(code) ou SemiPremier(code))
    indice ← indice + ConvCh(code) + " "
  Fin Pour

  Répéter
    Ecrire("Ordre ? ")
    Lire(ordre)
    ordch ← Convch(ordre div 10) + Convch(ordre mod 10)
  Jusqu'à (1 ≤ ordre ≤ 99) et Rechercher(indice + ordch, indices, n) = -1
  
  Retourner indice + ordch
Fin

TDOL
Objet : Type/Nature
indice, ordch : chaine
nv, ordre, code : entier

Fonction Rechercher(ch: chaine, t: tab, n: entier): entier
Début
  p ← -1
  i ← 0
  Tant Que i < n Et p = -1 Faire
    Si t[i] = ch Alors
      p ← i
    Fin Si
    i ← i + 1
  Fin Tant Que
  Retourner p
Fin

TDOL
Objet : Type/Nature
i, p : entier
test : booléen

Fonction Premier(n: entier): booléen
Début
  test ← n >= 2
  i ← 2
  Tant Que i * i ≤ n Et test Faire
    test ← n mod i ≠ 0
    i ← i + 1
  Fin Tant Que
  Retourner test
Fin

TDOL
Objet : Type/Nature
i : entier
test : booléen

Fonction SemiPremier(n: entier): booléen
Début
  test ← Faux
  i ← 2
  Tant Que i * i ≤ n Et non test Faire
    test ← n mod i = 0 et Premier(i) et Premier(n div i)
    i ← i + 1
  Fin Tant Que
  Retourner test
Fin

TDOL
Objet : Type/Nature
i : entier
test : booléen

Procédure Décrypter(indices: tab, @mots: tab, n: entier)
Début
  Pour i de 0 à n-1 Faire
    mots[i] ← DécrypterIndice(indices[i])
  Fin Pour
Fin

Fonction DécrypterIndice(indice: chaîne):chaine
Début
    ch ← ""
    Tant Que Long(indice) > 2 Faire
      p ← Pos(" ", indice)
      car ← chr(Valeur(Sous_chaine(indice, 0, p)))
      ch ← ch + car
      indice ← Sous_chaine(indice, p+1, Long(indice))
    Fin Tant Que
    Retourner ch + indice
Fin

TDOL
Objet : Type/Nature
p : Entier
ch : chaine
car : caractere

Procédure Classer(@mots: tab, n: Entier)
Début
  Répéter
    trié ← Vrai
    Pour i de 0 à n-2 Faire
      l1 ← Long(mots[i])
      l2 ← Long(mots[i+1])
      c1 ← mots[i][l1-2] + mots[i][l1-1]
      c2 ← mots[i+1][l2-2] + mots[i+1][l2-1]
      Si c1 > c2 Alors
        aux ← mots[i]
        mots[i] ← mots[i+1]
        mots[i+1] ← aux
        trié ← Faux
      Fin Si
    Fin Pour
  Jusqu'à trié
Fin

TDOL
Objet : Type/Nature
i, l1, l2 : Entier
c1, c2, aux : Chaine
trié : booleen

Fonction Reconstituer(@mots: tab, n: entier):chaine
Début
  ph ← ""
  Pour i de 0 à n-1 Faire
    ph ← ph + Sous_chaine(mots[i], 0, Long(mots[i]) - 2) + " "
  Fin Pour
  Retourner Sous_Chaine(ph, 0, Long(ph)-1)
Fin

Algorithme 02_algorithme_de_revision
Début
  Saisir(N)
  Remplir(indices, N)
  Décrypter(indices, mots, N)
  Classer(MOTS, N)
  Ecrire(Reconstituer(mots, N))
Fin

TDNT
tab = Tableau de 10 Chaine

TDO
Objet : Type/Nature
indices, mots : Tab
N : Entier
