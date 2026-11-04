Procédure Saisir_N (@N : Entier)
Début
    Répéter
        Ecrire("Donner le nombre de joueurs (entre 5 et 10) : ")
        Lire(N)
    Jusqu'à (N Dans [5..10])
Fin

Procédure FICHE_JOUEUR (@V : TAB, N : Entier)
Début
    Pour i de 0 à N - 1 Faire
        Ecrire("--- Saisie du joueur n°", i + 1, " ---")
        Répéter
            Ecrire("Donner l'identifiant (10 car. : 1ère/dernière majuscules, 8 chiffres) : ")
            Lire(V[i].ID)
            Ch <- V[i].ID
            Si Long(Ch) = 10 Alors
                C0 <- Ch[0]
                Cn <- Ch[Long(Ch) - 1]
                Reste <- SousChaine(Ch, 1, 8)
            Fin Si
        Jusqu'à (Long(Ch) = 10) ET (C0 Dans ["A".."Z"]) ET (Cn Dans ["A".."Z"]) ET EstNum(Reste)

        Pour j de 0 à 7 Faire
            Répéter
                Ecrire("Essai n°", j + 1, " (0 à 10 quilles) : ")
                Lire(V[i].ES[j])
            Jusqu'à (V[i].ES[j] Dans [0..10])
        Fin Pour
    Fin Pour
Fin

TDOL
Objet : Type/Nature
i, j : Entier
Ch, Reste : Chaine
C0, Cn : Caractère

Fonction Moyenne (T : RES) : Réel
Début
    S ← 0
    Pour i de 0 à 7 Faire
        S <- S + T[i]
    Fin Pour
    Retourner S / 8
Fin

TDOL
Objet : Type/Nature
S, i : Entier

Procédure TRI_FICHE (@V : TAB, N : Entier)
Début
    Répéter
        Ok <- Vrai
        Pour i de 0 à N - 2 Faire
            Si Moyenne(V[i].ES) < Moyenne(V[i + 1].ES) Alors
                Ok <- Faux
                Aux <- V[i]
                V[i] <- V[i + 1]
                V[i + 1] <- Aux
            Fin Si
        Fin Pour
    Jusqu'à Ok
Fin

TDOL
Objet : Type/Nature
i : Entier
Ok : Booléen
Aux : JOUEUR
Moyenne : Fonction

Procédure Afficher_Classement (V : TAB, N : Entier)
Début
    Ecrire_NL("===== CLASSEMENT DE LA COMPÉTITION =====")
    Pour i de 0 à N - 1 Faire
        Ecrire_NL("Rang ", i + 1, " : Joueur ", V[i].ID, " | Efficacité : ", Moyenne(V[i].ES))
    Fin Pour
    Ecrire_NL(">> Meilleur joueur : ", V[0].ID, " avec une moyenne de ", Moyenne(V[0].ES))
Fin

TDOL
Objet : Type/Nature
i : Entier
Moyenne : Fonction

Algorithme Bowling
Début
    Saisir_N(N)
    FICHE_JOUEUR(V, N)
    TRI_FICHE(V, N)
    Afficher_Classement(V, N)
Fin

TDNT
RES = Tableau de 8 Entier
JOUEUR = Enregistrement
    ID : Chaine
    ES : RES
Fin
TAB = Tableau de 10 JOUEUR

TDO
Objet : Type/Nature
V : TAB
N : Entier
Saisir_N : Procédure
FICHE_JOUEUR : Procédure
TRI_FICHE : Procédure
Afficher_Classement : Procédure
