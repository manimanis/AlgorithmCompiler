Procédure Saisir(@ch: chaîne)
Début
  Répéter
    Ecrire("Entrez une chaîne: ")
    Lire(ch)
  Jusqu'à verif(ch)
Fin

Fonction verif(ch: chaîne): booléen
Début
  test ← Long(ch) ≥ 0 et ch[0] ≠ " " et ch[Long(ch)-1] ≠ " " et Pos("  ", ch) = -1
  i ← 0
  nm ← 0
  Tant que i < Long(ch) et test faire
    test ← "A" ≤ majus(ch[i]) ≤ "Z" ou ch[i] = " "
    Si ch[i] = " " alors
      nm ← nm + 1
    Fin Si
    i ← i + 1
  Fin Tant que
  Retourner test et nm ≤ 20
Fin

TDOL
Objet : Type/Nature
test : booléen
i, nm : entier

Procédure Découper(@t: tab, @n:entier, ch: chaîne)
Début
  ch ← ch + " "
  n ← 0
  Tant que ch ≠ "" faire
    p ← Pos(" ", ch)
    mot ← sous_chaine(ch, 0, p)
    t[n] ← mot
    n ← n + 1
    ch ← effacer(ch, 0, p + 1)
  Fin Tant que
Fin

TDOL
Objet : Type/Nature
p : entier
mot : chaine

Procédure Crypter(@t: tab, n: entier)
Début
  Pour i de 0 à n-1 Faire
    Si (i+1) mod 2 = 0 Alors
      p ← i+1
    Sinon
      p ← -(i+1)
    Fin Si
    t[i] ← CrypterMot(t[i], p)
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i, p : entier

Fonction CrypterMot(mot: chaîne, p: entier): chaîne
Début
  res ← ""
  Pour i de 0 à Long(mot)-1 Faire
    c ← mot[i]
    Si "A" ≤ c ≤ "Z" Alors
      ref ← "A"
    Sinon
      ref ← "a"
    Fin Si
    c ← chr((ord(c) - ord(ref) + p) mod 26 + ord(ref))
    res ← res + c
  Fin Pour
  Retourner res
Fin

TDOL
Objet : Type/Nature
c, ref : caractère
i : entier
res : chaîne

Procédure Trier(@t: tab, n: entier)
Début
  // Tri par sélection
  Pour i de 0 à n-2 Faire
    min ← i
    Pour j de i+1 à n-1 Faire
      Si Long(t[j]) < Long(t[min]) Alors
        min ← j
      Fin Si
    Fin Pour
    Si min ≠ i Alors
      tmp ← t[i]
      t[i] ← t[min]
      t[min] ← tmp
    Fin Si
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i, j, min : entier
tmp : chaîne

Procédure Afficher(t: tab, n: entier)
Début
  Pour i de 0 à n-1 Faire
    Ecrire(t[i])
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i : entier

Algorithme 04_programme_de_cryptage
// Programme principal
Début  
  Saisir(ch)
  Découper(t, n, ch)
  Crypter(t, n)
  Trier(t, n)
  Afficher(t, n)
Fin

TDNT
tab = tableau de 20 chaîne

TDO
Objet : Type/Nature
t : tab
n : entier
ch : chaîne
