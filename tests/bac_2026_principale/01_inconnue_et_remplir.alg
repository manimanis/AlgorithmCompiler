Fonction Inconnue (a, b : Entier) : Entier
Début
    Si b = 0 Alors
        Retourner 0
    Sinon
        Si b Mod 2 = 0 Alors
            Retourner Inconnue(2 * a, b Div 2)
        Sinon
            Retourner a + Inconnue(a, b - 1)
        Fin Si
    Fin Si
Fin

Procédure Remplir (ch1, ch2 : Chaine)
Début
    Ouvrir(ch1, Ft, "r")
    Ouvrir(ch2, Fb, "wb")
    Tant Que Non Fin_Fichier(Ft) Faire
        Lire_Ligne(Ft, ch)
        p <- Pos(" ", ch)
        V.A <- Valeur(SousChaine(ch, 0, p))
        V.B <- Valeur(SousChaine(ch, p + 1, Long(ch)))
        V.R <- Inconnue(V.A, V.B)
        Ecrire(Fb, V)
    Fin Tant Que
    Fermer(Ft)
    Fermer(Fb)
Fin

TDOL
Objet : Type/Nature
Ft : Fichier texte
Fb : Fichier de Enreg
V : Enreg
ch : Chaine
p : Entier
Inconnue : Fonction

Algorithme Exercice1_Bac2026
Début
    ch1 <- "donnees_ex1.txt"
    ch2 <- "resultats_ex1.dat"

    // Création du fichier source ch1
    Ouvrir(ch1, F_init, "w")
    Ecrire_NL(F_init, "8 7")
    Ecrire_NL(F_init, "10 6")
    Ecrire_NL(F_init, "3 5")
    Fermer(F_init)

    // Appel de la procédure Remplir corrigée
    Remplir(ch1, ch2)

    // Vérification du fichier binaire produit ch2
    Ouvrir(ch2, Fb_verif, "rb")
    Tant Que Non Fin_Fichier(Fb_verif) Faire
        Lire(Fb_verif, e)
        Ecrire_NL("A = ", e.A, " | B = ", e.B, " | R = ", e.R)
    Fin Tant Que
    Fermer(Fb_verif)
Fin

TDNT
Enreg = Enregistrement
    A : Entier
    B : Entier
    R : Entier
Fin

TDO
Objet : Type/Nature
ch1, ch2 : Chaine
F_init : Fichier texte
Fb_verif : Fichier de Enreg
e : Enreg
Remplir : Procédure
Inconnue : Fonction
