Procédure Saisir_Epsilon (@eps : Réel)
Début
    Répéter
        Ecrire("Donner la précision epsilon (ex: 0.0001) : ")
        Lire(eps)
    Jusqu'à (eps > 0) ET (eps < 1)
Fin

Fonction Prod (x : Entier) : Entier
Début
    Si x <= 2 Alors
        Retourner x
    Sinon
        Retourner x * Prod(x - 2)
    Fin Si
Fin

Fonction puis (x, y : Entier) : Entier
Début
    p <- 1
    Pour j de 1 à y Faire
        p <- p * x
    Fin Pour
    Retourner p
Fin

TDOL
Objet : Type/Nature
j, p : Entier

Fonction CalculPi (epsilon : Réel) : Réel
Début
    i <- 1
    s <- 1 / 2
    Répéter
        sp <- s
        terme <- (Prod(i) / Prod(i + 1)) * (1 / ((i + 2) * puis(2, i + 2)))
        s <- sp + terme
        i <- i + 2
    Jusqu'à Abs(s - sp) <= epsilon
    Retourner 6 * s
Fin

TDOL
Objet : Type/Nature
i : Entier
s, sp, terme : Réel
Prod : Fonction
puis : Fonction

Algorithme Approximation_Pi
Début
    Saisir_Epsilon(epsilon)
    pi_approx <- CalculPi(epsilon)
    Ecrire_NL("Valeur approchée de Pi : ", pi_approx)
Fin

TDO
Objet : Type/Nature
epsilon : Réel
pi_approx : Réel
Saisir_Epsilon : Procédure
CalculPi : Fonction
