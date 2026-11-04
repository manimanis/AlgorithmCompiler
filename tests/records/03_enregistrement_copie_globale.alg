Algorithme test_rec_copie
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
