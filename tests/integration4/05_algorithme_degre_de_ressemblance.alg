Procédure SaisirCh(@ch: chaîne)
Début
  Répéter
    Ecrire("Entrez un mot : ")
    Lire(ch)
  Jusqu'à Long(ch) > 0 et verif(ch)
Fin

Procédure Saisir(@n: entier)
Début
  Répéter
    Ecrire("Nombre de mots : ");
    Lire(n);
  Jusqu'à 5 ≤ n ≤ 10
Fin

Procédure Remplir(ch: chaîne, @t: tab1, n: entier)
Début
  Pour i de 0 à n-1 Faire
    Répéter
      Ecrire("t[", i, "] = ")
      Lire(t[i])
    Jusqu'à Long(t[i]) = Long(ch) et verif(t[i])
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i : entier

Fonction verif(ch: chaîne):booléen
Début
  test ← Vrai
  i ← 0
  Tant Que test et i < Long(ch) Faire
    test ← "A" ≤ ch[i] ≤ "Z"
    i ← i + 1
  Fin Tant Que
  Retourner test
Fin

TDOL
Objet : Type/Nature
i : entier
test : booléen

Procédure Générer(@D: tab2, ch: chaîne, t: tab1, n: entier)
Début
  Pour i de 0 à n-1 Faire
    D[i] ← DegréRess(t[i], ch)
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i : entier

Fonction DegréRess(ch1: chaîne, ch2: chaîne):réel
Début
  nc ← 0
  Pour i de 0 à Long(ch1)-1 Faire
    nc ← nc + (ch1[i] = ch2[i])
  Fin Pour
  Retourner nc * 100 / Long(ch1)
Fin

TDOL
Objet : Type/Nature
i, nc : entier
r : réel

Fonction Max(d: tab2, n: entier):réel
Début
  mx ← d[0]
  Pour i de 1 à n-1 Faire
    Si d[i] > mx Alors
      mx ← d[i]
    Fin Si
  Fin Pour
  Retourner mx
Fin

TDOL
Objet : Type/Nature
mx : réel
i : entier

Procédure Afficher(dr: réel, t: tab1, d: tab2, n: entier)
Début
  Ecrire("Mots ayan le DR maximal : ", fin="")
  Pour i de 0 à n-1 Faire
    Si d[i] = dr Alors
      Ecrire(t[i], fin=", ")
    Fin Si
  Fin Pour
  Ecrire()
Fin

TDOL
Objet : Type/Nature
i : entier

Algorithme Degré_Ressemblance
Début
  SaisirCh(ch)
  Saisir(n)
  Remplir(ch, t, n)
  Générer(d, ch, t, n)
  drm ← Max(d, n)
  Ecrire("DR maximal :", Arrondi(drm), "%")
  Afficher(drm, t, d, n)
Fin

TDNT
tab1 = tableau de 10 chaîne
tab2 = tableau de 10 réel

TDO
Objet : Type/Nature
ch : chaîne
t : tab1
d : tab2
n : entier
drm : réel
