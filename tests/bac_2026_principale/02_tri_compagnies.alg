Procédure Tri (Ch1, Ch2 : Chaine)
Début
    Ouvrir(Ch1, Fa, "rb")
    N <- 0
    Tant Que Non Fin_Fichier(Fa) Faire
        Lire(Fa, T[N])
        N <- N + 1
    Fin Tant Que
    Fermer(Fa)

    // Tri à bulles décroissant selon NSat
    Répéter
        Permut <- Faux
        Pour i de 0 à N - 2 Faire
            Si T[i].NSat < T[i + 1].NSat Alors
                Aux <- T[i]
                T[i] <- T[i + 1]
                T[i + 1] <- Aux
                Permut <- Vrai
            Fin Si
        Fin Pour
    Jusqu'à Non Permut

    Ouvrir(Ch2, Fc, "w")
    Pour i de 0 à N - 1 Faire
        Ecrire_NL(Fc, T[i].Comp, " ", T[i].NSat, " ", T[i].NInsat)
    Fin Pour
    Fermer(Fc)
Fin

TDOL
Objet : Type/Nature
Fa : Fichier de EnregComp
Fc : Fichier texte
T : TabComp
Aux : EnregComp
N, i : Entier
Permut : Booléen

Algorithme Exercice2_Tri_Compagnies
Début
    Ch1 <- "compagnies.dat"
    Ch2 <- "classement_compagnies.txt"

    // Création du fichier d'enregistrements Fa
    Ouvrir(Ch1, Fa_init, "wb")
    c1.Comp <- "Tunisair"   ; c1.NSat <- 1200 ; c1.NInsat <- 800  ; Ecrire(Fa_init, c1)
    c2.Comp <- "AirFrance"  ; c2.NSat <- 4500 ; c2.NInsat <- 500  ; Ecrire(Fa_init, c2)
    c3.Comp <- "Emirates"   ; c3.NSat <- 8900 ; c3.NInsat <- 200  ; Ecrire(Fa_init, c3)
    c4.Comp <- "Lufthansa"  ; c4.NSat <- 3100 ; c4.NInsat <- 600  ; Ecrire(Fa_init, c4)
    Fermer(Fa_init)

    // Appel du module Tri
    Tri(Ch1, Ch2)

    // Affichage du résultat texte
    Ouvrir(Ch2, Fc_aff, "r")
    Tant Que Non Fin_Fichier(Fc_aff) Faire
        Lire_Ligne(Fc_aff, ligne)
        Ecrire_NL(ligne)
    Fin Tant Que
    Fermer(Fc_aff)
Fin

TDNT
EnregComp = Enregistrement
    Comp : Chaine
    NSat : Entier
    NInsat : Entier
Fin
TabComp = Tableau de 100 EnregComp

TDO
Objet : Type/Nature
Ch1, Ch2, ligne : Chaine
Fa_init : Fichier de EnregComp
Fc_aff : Fichier texte
c1, c2, c3, c4 : EnregComp
Tri : Procédure
