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

Procédure SaisirCh(@ch: chaîne)
Début
  Répéter
    Ecrire("Entrez un mot : ")
    Lire(ch)
  Jusqu'à Long(ch) > 0 et verif(ch)
Fin

Algorithme 04_fonction_verif_et_saisie
Début
  SaisirCh(ch)
  x ← verif(ch)
  Ecrire(x)
Fin

TDO
Objet : Type/Nature
ch : chaîne
x : booléen
