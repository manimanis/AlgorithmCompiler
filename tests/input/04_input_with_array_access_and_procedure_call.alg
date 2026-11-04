Procédure Remplir(@t: tab)
Début
  Pour i de 0 à 4 faire
    Répéter
      Lire(t[i])
    Jusqu'à Recherche(t[i], t, i) = -1
  Fin Pour
Fin

TDOL
Objet : Type/Nature
i : entier

Fonction Recherche(x: entier, t: tab, n: entier): entier
Début
  Pour i de 0 à n-1 faire
    Si t[i] = x alors
      Retourner i
    Fin Si
  Fin Pour
  Retourner -1
Fin

TDOL
Objet : Type/Nature
i : entier

Algorithme 04_input_with_array_access_and_procedure_call
Début
  Remplir(t)
  Pour i de 0 à 4 faire
    Ecrire(t[i])
  Fin Pour
Fin

TDNT
tab = tableau de 5 entier

TDO
Objet : Type/Nature
t : tab
i : entier
