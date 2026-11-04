Procédure Saisir(@n: entier)
Début
  Répéter
    Ecrire("Entrez un entier : ");
    Lire(n);
  Jusqu'à 0 < n ≤ 20
Fin

Procédure Remplir(@t: tab, n: entier)
Début
  Pour i de 0 à n-1 Faire
    Ecrire("t[", i, "] = ");
    Lire(t[i]);
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i : entier

Procédure Copier(@t2: tab, t1: tab, n: entier)
Début
  Pour i de 0 à n-1 Faire
    t2[i] ← t1[i]
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i : entier

Procédure Afficher(t: tab, n: entier)
Début
  Pour i de 0 à n-1 Faire
    Ecrire(t[i], fin=" ");
  Fin Pour
  Ecrire()
Fin

TDOL
Objet : Type/Nature
i : entier

Procédure TriSelection(@t: tab, n: entier)
Début
  Pour i de 0 à n-2 Faire
    min ← i
    Pour j de i+1 à n-1 Faire
      Si t[j] < t[min] Alors
        min ← j
      Fin Si
    Fin Pour
    Si min ≠ i Alors
      temp ← t[i]
      t[i] ← t[min]
      t[min] ← temp
    Fin Si
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i, j, min, temp : entier

Procédure TriInsertion(@t: tab, n: entier)
Début
  Pour i de 1 à n-1 Faire
    temp ← t[i]
    j ← i - 1
    Tant Que j >= 0 Et t[j] > temp Faire
      t[j+1] ← t[j]
      j ← j - 1
    Fin Tant Que
    t[j+1] ← temp
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i, j, temp : entier
trié : booléen

Procédure TriBulles(@t: tab, n: entier)
Début
  Répéter
    trié ← Vrai
    Pour i de 0 à n-2 Faire
      Si t[i] > t[i+1] Alors
        temp ← t[i]
        t[i] ← t[i+1]
        t[i+1] ← temp
        trié ← Faux
      Fin Si
    Fin Pour
  Jusqu'à trié
Fin

TDOL
Objet : Type/Nature
i, j, temp : entier
trié : booléen

Algorithme 03_tri_d_un_tableau
Début
  Saisir(n)
  Remplir(t1, n)
  Copier(t2, t1, n)
  Copier(t3, t1, n)
  Copier(t4, t1, n)
  
  TriBulles(t2, n)
  TriSelection(t3, n)
  TriInsertion(t4, n)
  
  Afficher(t2, n)
  Afficher(t3, n)
  Afficher(t4, n)
Fin

TDNT
tab = tableau de 20 entier

TDO
Objet : Type/Nature
t1, t2, t3, t4 : tab
n : entier
