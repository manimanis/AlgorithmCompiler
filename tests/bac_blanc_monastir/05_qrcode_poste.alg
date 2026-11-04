Procédure Init_Table (@T : TAB1)
Début
    ch_alphanumeric <- "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:"
    Pour i de 0 à 44 Faire
        T[i] <- ch_alphanumeric[i]
    Fin Pour
Fin

TDOL
Objet : Type/Nature
i : Entier
ch_alphanumeric : Chaine

Fonction Conv10_2 (n : Entier) : Chaine
Début
    Si n = 0 Alors
        Retourner ""
    Sinon
        Retourner Conv10_2(n Div 2) + ConvCh(n Mod 2)
    Fin Si
Fin

Fonction Ajouter_Zero (bin : Chaine, nb : Entier) : Chaine
Début
    Tant Que Long(bin) < nb Faire
        bin <- "0" + bin
    Fin Tant Que
    Retourner bin
Fin

Fonction Recherche (c : Caractère, T : TAB1) : Entier
Début
    i <- 0
    Tant Que (i <= 44) ET (T[i] <> c) Faire
        i <- i + 1
    Fin Tant Que
    Retourner i
Fin

TDOL
Objet : Type/Nature
i : Entier

Fonction Ajouter_Indicateurs (CHb : Chaine, L : Entier) : Chaine
Début
    bin <- Conv10_2(L)
    bin <- Ajouter_Zero(bin, 9)
    Retourner "0010" + bin + CHb + "0000"
Fin

TDOL
Objet : Type/Nature
bin : Chaine
Conv10_2 : Fonction
Ajouter_Zero : Fonction

Fonction Ajouter_Padding (CHb : Chaine) : Chaine
Début
    Tant Que Long(CHb) Mod 8 <> 0 Faire
        CHb <- CHb + "0"
    Fin Tant Que
    Retourner CHb
Fin

Fonction Conversion (CH : Chaine, T : TAB1) : Chaine
Début
    L <- Long(CH)
    CHb <- ""
    Tant Que Long(CH) >= 2 Faire
        c <- SousChaine(CH, 0, 2)
        v <- Recherche(c[0], T) * 45 + Recherche(c[1], T)
        bin <- Conv10_2(v)
        bin <- Ajouter_Zero(bin, 11)
        CHb <- CHb + bin
        CH <- Effacer(CH, 0, 2)
    Fin Tant Que

    Si CH <> "" Alors
        v <- Recherche(CH[0], T)
        bin <- Conv10_2(v)
        bin <- Ajouter_Zero(bin, 6)
        CHb <- CHb + bin
    Fin Si

    CHb <- Ajouter_Indicateurs(CHb, L)
    CHb <- Ajouter_Padding(CHb)
    Retourner CHb
Fin

TDOL
Objet : Type/Nature
L, v : Entier
CHb, bin, c : Chaine
Recherche : Fonction
Conv10_2 : Fonction
Ajouter_Zero : Fonction
Ajouter_Indicateurs : Fonction
Ajouter_Padding : Fonction
Effacer : Fonction

Fonction Conv2_10 (bin : Chaine) : Entier
Début
    N <- 0
    p <- 1
    Pour i de 7 à 0 [Pas = -1] Faire
        Si bin[i] = "1" Alors
            N <- N + p
        Fin Si
        p <- p * 2
    Fin Pour
    Retourner N
Fin

TDOL
Objet : Type/Nature
N, p, i : Entier

Procédure CorrectionErreur (CHb : Chaine, @P : TAB2)
Début
    N <- 0
    Tant Que CHb <> "" Faire
        P[N] <- Conv2_10(SousChaine(CHb, 0, 8))
        N <- N + 1
        CHb <- SousChaine(CHb, 8, Long(CHb) - 8)
    Fin Tant Que

    alt <- 0
    Pour i de N à 15 Faire
        Si alt Mod 2 = 0 Alors
            P[i] <- 236
        Sinon
            P[i] <- 17
        Fin Si
        alt <- alt + 1
    Fin Pour
Fin

TDOL
Objet : Type/Nature
N, i, alt : Entier
Conv2_10 : Fonction

Procédure Former (@ft : Fichier texte, @fb : FichB, T : TAB1)
Début
    Ouvrir("Expéditeurs.txt", ft, "r")
    Ouvrir("Archive_Globale.dat", fb, "wb")

    Tant Que Non Fin_Fichier(ft) Faire
        Lire_Ligne(ft, CH)
        CH_Crypte <- Conversion(CH, T)
        CorrectionErreur(CH_Crypte, P)
        e.Info_Crypte <- CH_Crypte
        Pour i de 0 à 15 Faire
            e.P[i] <- P[i]
        Fin Pour
        Ecrire(fb, e)
    Fin Tant Que

    Fermer(ft)
    Fermer(fb)
Fin

TDOL
Objet : Type/Nature
CH, CH_Crypte : Chaine
i : Entier
e : Enreg
P : TAB2
Conversion : Fonction
CorrectionErreur : Procédure

Algorithme QRCode_Poste
Début
    Ouvrir("Expéditeurs.txt", ft, "w")
    Ecrire_NL(ft, "TUNIS 2024")
    Ecrire_NL(ft, "POSTE TN")
    Fermer(ft)

    Init_Table(T)
    Former(ft, fb, T)
    Ecrire_NL("Génération et archivage des QR Codes terminés avec succès.")
Fin

TDNT
TAB1 = Tableau de 45 Caractère
TAB2 = Tableau de 16 Entier
Enreg = Enregistrement
    Info_Crypte : Chaine
    P : TAB2
Fin
FichB = Fichier de Enreg

TDO
Objet : Type/Nature
T : TAB1
ft : Fichier texte
fb : FichB
Init_Table : Procédure
Former : Procédure
