Procédure Saisie_Param (@x, @K : Entier)
Début
    Répéter
        Ecrire("Donner x (x > 1) : ")
        Lire(x)
    Jusqu'à (x > 1)

    Répéter
        Ecrire("Donner le nombre de termes K (K >= 1) : ")
        Lire(K)
    Jusqu'à (K >= 1)
Fin

Fonction Terme (x, n : Entier) : Entier
Début
    Si n = 0 Alors
        Retourner x
    Sinon
        Retourner 3 * Terme(x, n - 1) - 2
    Fin Si
Fin

Procédure Remplir (@F : Fichier texte, K, x : Entier)
Début
    Ouvrir("source.txt", F, "w")
    Pour i de 0 à K - 1 Faire
        Ecrire_NL(F, ConvCh(Terme(x, i)))
    Fin Pour
    Fermer(F)
Fin

TDOL
Objet : Type/Nature
i : Entier
Terme : Fonction

Fonction Puissance (x, y : Entier) : Entier
Début
    P <- 1
    Pour i de 1 à y Faire
        P <- P * x
    Fin Pour
    Retourner P
Fin

TDOL
Objet : Type/Nature
i, P : Entier

Procédure Facteurs (n : Entier, @T : Tab, @nb : Entier)
Début
    i <- 2
    nb <- 0
    Répéter
        Si n Mod i = 0 Alors
            p <- 0
            Tant Que n Mod i = 0 Faire
                p <- p + 1
                n <- n Div i
            Fin Tant Que
            T[nb].fact <- i
            T[nb].puis <- p
            nb <- nb + 1
        Sinon
            i <- i + 1
        Fin Si
    Jusqu'à (n = 1)
Fin

TDOL
Objet : Type/Nature
i, p : Entier

Procédure Determiner (n : Entier, @a, @b : Entier)
Début
    Facteurs(n, T, nb)
    a <- 1
    b <- 1
    Pour i de 0 à nb - 1 Faire
        a <- a * Puissance(T[i].fact, T[i].puis Div 2)
        Si T[i].puis Mod 2 <> 0 Alors
            b <- b * T[i].fact
        Fin Si
    Fin Pour
Fin

TDOL
Objet : Type/Nature
T : Tab
nb, i : Entier
Facteurs : Procédure
Puissance : Fonction

Procédure Simplifier (@F : Fichier texte)
Début
    Ouvrir("source.txt", F, "r")
    Tant Que Non Fin_Fichier(F) Faire
        Lire_Ligne(F, ch)
        n <- Valeur(ch)
        Determiner(n, a, b)
        Si (a <> 1) ET (b <> 1) Alors
            Ecrire_NL("Racine(", ch, ") = ", a, " * Racine(", b, ")")
        Sinon Si (a = 1) ET (b <> 1) Alors
            Ecrire_NL("Racine(", ch, ") = Racine(", b, ")")
        Sinon Si (b = 1) Alors
            Ecrire_NL("Racine(", ch, ") = ", a)
        Fin Si
    Fin Tant Que
    Fermer(F)
Fin

TDOL
Objet : Type/Nature
ch : Chaine
a, b, n : Entier
Determiner : Procédure

Algorithme Racines_Suite
Début
    Saisie_Param(x, K)
    Remplir(F, K, x)
    Simplifier(F)
Fin

TDNT
facteur = Enregistrement
    fact : Entier
    puis : Entier
Fin
Tab = Tableau de 100 facteur

TDO
Objet : Type/Nature
F : Fichier texte
x, K : Entier
Saisie_Param : Procédure
Remplir : Procédure
Simplifier : Procédure
