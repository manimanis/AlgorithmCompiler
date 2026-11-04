Algorithme Exercice6
Début
  Ecrire("Donner a ? ") ; lire(a)
  Ecrire("Donner b ? ") ; lire(b)
  Si a ≠ 0 Alors
    x ← -b / a
    Ecrire("Solution :", x)
  Sinon
    Si b = 0 Alors
      Ecrire("S = IR")
    Sinon
      Ecrire("S = {}")
    Fin Si
  Fin Si
Fin

TDO
Objet : Type/Nature
a,b,x : réel
