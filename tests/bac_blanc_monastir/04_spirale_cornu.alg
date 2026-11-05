// @max-iter: 20000000
// @timeout: 120000
Fonction ApproximationPi (N : Entier) : Réel
Début
  E <- 1
  M[0, 0] <- 1
  Pour i de 1 à N - 1 Faire
    Ep <- E
    Si i Mod 2 <> 0 Alors
      M[i, 0] <- 0
      Pour j de 1 à i Faire
        M[i, j] <- M[i, j - 1] + M[i - 1, j - 1]
      Fin Pour
      E <- M[i, i]
    Sinon
      M[i, i] <- 0
      Pour j de i - 1 à 0 [Pas = -1] Faire
        M[i, j] <- M[i, j + 1] + M[i - 1, j]
      Fin Pour
      E <- M[i, 0]
    Fin Si
  Fin Pour
  Retourner (2 * i * Ep) / E
Fin

TDOL
Objet : Type/Nature
M : Matrice_Seidel
E, Ep, i, j : Entier

Fonction PI (n : Entier) : Réel
Début
  Retourner ApproximationPi(n)
Fin

Fonction sin (X : Réel) : Réel
Début
  P2 <- X
  N <- 1
  Répéter
    P1 <- P2
    P2 <- P2 * (1 - ((X * X) / (N * N * mypi * mypi)))
    N <- N + 1
  Jusqu'à Abs(P2 - P1) <= 0.000001
  Retourner P2
Fin

TDOL
Objet : Type/Nature
P1, P2 : Réel
N : Entier

Fonction f(X : Réel) : Réel
Début
  Retourner sin((mypi * X * X) / 2)
Fin

TDOL
Objet : Type/Nature
sin, PI : Fonction

Procédure Point_Raccordement (@x1, @x2 : Réel, eps : Réel)
Début
  x1 <- 0.2
  Tant Que Abs(f(x1) - x1) > eps Faire
    x1 <- x1 + eps
  Fin Tant Que

  x2 <- 1.2
  Tant Que Abs(f(x2) - x2) > eps Faire
    x2 <- x2 - eps
  Fin Tant Que
Fin

TDOL
Objet : Type/Nature
f : Fonction

Fonction Somme (x1, H : Réel, i : Entier) : Réel
Début
  Si i = 1 Alors
    Retourner f(x1 + H)
  Sinon
    Retourner f(x1 + i * H) + Somme(x1, H, i - 1)
  Fin Si
Fin

TDOL
Objet : Type/Nature
f : Fonction

Procédure Aire (x1, x2 : Réel, N : Entier)
Début
  H <- (x2 - x1) / N
  resultat <- (H / 2) * (f(x1) + f(x2) + 2 * Somme(x1, H, N - 1))
  Ecrire_NL("L'aire sous la courbe entre x1 et x2 est : ", resultat)
Fin

TDOL
Objet : Type/Nature
H, resultat : Réel
f, Somme : Fonction

Algorithme Clothoide_Ferroviaire
Début
  eps <- 0.0001
  mypi ← PI(9)
  Point_Raccordement(x1, x2, eps)
  Ecrire_NL("Point fixe x1 = ", x1)
  Ecrire_NL("Point fixe x2 = ", x2)
  Répéter
    Ecrire("Donner le nombre de trapèzes N (N >= 10) : ")
    Lire(N)
  Jusqu'à (N >= 10)
  Aire(x1, x2, N)
Fin

TDNT
Matrice_Seidel = Tableau [0..20, 0..20] de Entier

TDO
Objet : Type/Nature
x1, x2, eps, mypi : Réel
N : Entier
Point_Raccordement : Procédure
Aire : Procédure
