Procédure Saisie (@Vi, @Vf, @P : Entier)
Début
    Répéter
        Ecrire("Donner la borne inférieure Vi (Vi >= 2) : ")
        Lire(Vi)
        Ecrire("Donner la borne supérieure Vf (Vf > Vi) : ")
        Lire(Vf)
    Jusqu'à (Vi >= 2) ET (Vf > Vi)

    Répéter
        Ecrire("Donner le pas P (P >= 1) : ")
        Lire(P)
    Jusqu'à (P >= 1)
Fin

Fonction Force (X : Entier) : Entier
Début
    i <- 2
    ip <- 0
    prod <- 1
    som <- 0
    Tant Que X <> 1 Faire
        Tant Que X Mod i <> 0 Faire
            i <- i + 1
        Fin Tant Que
        Si i <> ip Alors
            prod <- prod * i
            ip <- i
        Fin Si
        som <- som + 1
        X <- X Div i
    Fin Tant Que
    Retourner som * prod
Fin

TDOL
Objet : Type/Nature
i, ip : Entier
prod, som : Entier

Procédure Recherche_Harmonique (Vi, Vf, P : Entier, @Xmax, @Fmax : Entier)
Début
    Xmax <- Vi
    Fmax <- Force(Xmax)
    Pour X de Vi + P à Vf [Pas = P] Faire
        Si Force(X) > Fmax Alors
            Xmax <- X
            Fmax <- Force(X)
        Fin Si
    Fin Pour
Fin

TDOL
Objet : Type/Nature
X : Entier
Force : Fonction

Algorithme Exercice1_Harmonique
Début
    Saisie(Vi, Vf, P)
    Recherche_Harmonique(Vi, Vf, P, Xmax, Fmax)
    Ecrire_NL("Le nombre harmonique est : ", Xmax, " avec une Force = ", Fmax)
Fin

TDO
Objet : Type/Nature
Vi, Vf, P : Entier
Xmax, Fmax : Entier
Saisie : Procédure
Recherche_Harmonique : Procédure
Force : Fonction
