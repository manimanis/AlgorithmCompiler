Procédure Init_Gamme (@G : TAB2)
Début
    G[0].note <- "Do" ; G[0].Ecart <- 0
    G[1].note <- "Ré" ; G[1].Ecart <- 2
    G[2].note <- "Mi" ; G[2].Ecart <- 4
    G[3].note <- "Sol"; G[3].Ecart <- 7
    G[4].note <- "La" ; G[4].Ecart <- 9
Fin

Fonction Recherche (X : Entier, T : TAB_Entier, N : Entier) : Booléen
Début
    i <- 0
    Tant Que (i < N) ET (T[i] <> X) Faire
        i <- i + 1
    Fin Tant Que
    Retourner (i < N)
Fin

TDOL
Objet : Type/Nature
i : Entier

Fonction Suite (k : Entier) : Entier
Début
    T[0] <- 0
    Pour i de 1 à k Faire
        d <- T[i - 1] - i
        Si (d > 0) ET Non Recherche(d, T, i) Alors
            T[i] <- d
        Sinon
            T[i] <- T[i - 1] + i
        Fin Si
    Fin Pour
    Retourner T[k]
Fin

TDOL
Objet : Type/Nature
T : TAB_Entier
i, d : Entier
Recherche : Fonction

Fonction PUISS (X : Réel, Y : Réel) : Réel
Début
    Retourner Exp(Y * Ln(X))
Fin

Procédure Generer (@S : TAB1, N : Entier, G : TAB2)
Début
    Pour i de 0 à N - 1 Faire
        A <- Suite(i)
        S[i].Terme <- A
        P <- A Mod 5
        S[i].Note <- G[P].note
        S[i].Freq <- 261.63 * PUISS(2, G[P].Ecart / 12)
    Fin Pour
Fin

TDOL
Objet : Type/Nature
i, A, P : Entier
Suite : Fonction
PUISS : Fonction

Procédure Trier_Sequence (@S : TAB1, N : Entier)
Début
    P <- 0
    Tant Que P < N Faire
        P <- 3 * P + 1
    Fin Tant Que
    Tant Que P > 0 Faire
        P <- (P - 1) Div 3
        Pour i de P à N - 1 Faire
            Temp <- S[i]
            j <- i
            Tant Que (j >= P) ET (S[j - P].Freq > Temp.Freq) Faire
                S[j] <- S[j - P]
                j <- j - P
            Fin Tant Que
            S[j] <- Temp
        Fin Pour
    Fin Tant Que
Fin

TDOL
Objet : Type/Nature
i, j, P : Entier
Temp : Element_S

Procédure Afficher_Sequence (S : TAB1, N : Entier)
Début
    Ecrire_NL("Séquence musicale ordonnée (de la plus grave à la plus aiguë) :")
    Pour i de 0 à N - 1 Faire
        Ecrire_NL("Note : ", S[i].Note, " (Terme=", S[i].Terme, ") - Fréquence : ", S[i].Freq, " Hz")
    Fin Pour
Fin

TDOL
Objet : Type/Nature
i : Entier

Algorithme Musique_Recaman
Début
    Init_Gamme(G)
    Répéter
        Ecrire("Donner le nombre de notes N (1 <= N <= 100) : ")
        Lire(N)
    Jusqu'à (N >= 1) ET (N <= 100)
    Generer(S, N, G)
    Trier_Sequence(S, N)
    Afficher_Sequence(S, N)
Fin

TDNT
TAB_Entier = Tableau de 100 Entier
Note_Gamme = Enregistrement
    note : Chaine
    Ecart : Entier
Fin
TAB2 = Tableau de 5 Note_Gamme
Element_S = Enregistrement
    Terme : Entier
    Note : Chaine
    Freq : Réel
Fin
TAB1 = Tableau de 100 Element_S

TDO
Objet : Type/Nature
G : TAB2
S : TAB1
N : Entier
Init_Gamme : Procédure
Generer : Procédure
Trier_Sequence : Procédure
Afficher_Sequence : Procédure
