Procédure Remplir(@td: tab1, @te: tab2, n: entier)
Début
  Pour i de 1 à n Faire
    Ecrire("Drone n°", i)
    Répéter
      Ecrire("Nom ? ")
      Lire(td[i])
    Jusqu'à distinct(td[i], td, i) et verif_nom(td[i])

    dp ← aléa(10, 1000)
    Répéter
      Ecrire("Puissance ? ")
      Lire(p)
    Jusqu'à p ≥ 0
    te[i] ← dp * p
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i, dp : entier
p : réel

Fonction distinct(ch: chaine, t: tab1, n: entier):booléen
Début
  test ← Vrai
  i ← 0
  Tant Que test et i < n Faire
    test ← ch ≠ t[i]
    i ← i + 1
  Fin Tant Que
  Retourner test
Fin

TDOL
Objet : Type/Nature
i : entier
test : booléen

Fonction verif_nom(ch: chaîne):booléen
Début
  Retourner (9 ≤ Long(ch) ≤ 11) et Sous_chaine(ch, 0, 6) = "DRONE-" et EstNum(Sous_chaine(ch, 6, Long(ch)))
Fin

Procédure Supprimer(@td: tab1, @te: tab2, @n: entier)
Début
  i ← 1
  Pour j de 1 à n Faire
    Si te[j] ≠ 0 Alors
      te[i] ← te[j]
      td[i] ← td[j]
      i ← i + 1
    Fin Si
  Fin Pour
  n ← i - 1
Fin

TDOL
Objet : Type/Nature
i, j : entier

Procédure Trier(@td: tab1, @te: tab2, n: entier)
Début
  Répéter
    trié ← Vrai
    Pour i de 1 à n-1 Faire
      Si (te[i] < te[i+1]) ou (te[i] = te[i+1] et td[i] > td[i+1]) Alors
        aux1 ← te[i] ; te[i] ← te[i+1] ; te[i+1] ← aux1
        aux2 ← td[i] ; td[i] ← td[i+1] ; td[i+1] ← aux2
        trié ← Faux
      Fin Si
    Fin Pour
  Jusqu'à trié
Fin

TDOL
Objet : Type/Nature
i : entier
trié : booléen
aux1 : réel
aux2 : chaîne

Procédure Afficher(td: tab1, te: tab2, n: entier)
Début
  Pour i de 1 à n Faire
    Ecrire(td[i], ":", te[i])
  Fin Pour
Fin

Procédure Saisir(@n: entier)
Début
  Répéter
    Ecrire("n ? ")
    Lire(n)
  Jusqu'à 2 ≤ n ≤ 10
Fin

Algorithme 01_algorithme_drones
Début
  Saisir(n)
  Remplir(td, te, n)
  Supprimer(td, te, n)
  Trier(td, te, n)
  Afficher(td, te, n)
Fin

TDNT
tab1 = tableau de 11 chaîne
tab2 = tableau de 11 entier

TDO
Objet : Type/Nature
n : entier
td : tab1
te : tab2
