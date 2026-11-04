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

Algorithme Exercice3_Seidel
Début
    Répéter
        Ecrire("Donner le rang N pour la matrice de Seidel (ex: 9) : ")
        Lire(N)
    Jusqu'à (N >= 2) ET (N <= 20)
    pi_val <- ApproximationPi(N)
    Ecrire_NL("Valeur approchée de Pi : ", pi_val)
Fin

TDNT
Matrice_Seidel = Tableau [0..20, 0..20] de Entier

TDO
Objet : Type/Nature
N : Entier
pi_val : Réel
ApproximationPi : Fonction
