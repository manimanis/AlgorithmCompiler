Procédure Saisir_C (@C : Entier)
Début
    Répéter
        Ecrire("Donner le nombre de bâtiments C (1 <= C < 100) : ")
        Lire(C)
    Jusqu'à (C >= 1) ET (C < 100)
Fin

Procédure Entete (@M : Mat, C : Entier)
Début
    M[0, 0] <- ">>"
    Pour i de 1 à C Faire
        Répéter
            Ecrire("Donner le code du bâtiment ", i, " (au moins 3 chiffres) : ")
            Lire(M[0, i])
        Jusqu'à (EstNum(M[0, i])) ET (Long(M[0, i]) >= 3)
    Fin Pour

    base <- 8
    Pour i de 1 à 8 Faire
        Si base = 10 Alors
            base <- base + 1
        Fin Si
        M[i, 0] <- ConvCh(base)
        base <- base + 1
    Fin Pour
Fin

TDOL
Objet : Type/Nature
i, base : Entier

Fonction Convertir (x, base : Entier) : Chaine
Début
    ch <- ""
    Répéter
        r <- x Mod base
        Si r < 10 Alors
            ch <- ConvCh(r) + ch
        Sinon
            ch <- CHR(r + 55) + ch
        Fin Si
        x <- x Div base
    Jusqu'à (x = 0)
    Retourner ch
Fin

TDOL
Objet : Type/Nature
r : Entier
ch : Chaine

Procédure Contenu (@M : Mat, C : Entier)
Début
    Pour i de 1 à 8 Faire
        Pour j de 1 à C Faire
            M[i, j] <- Convertir(Valeur(M[0, j]), Valeur(M[i, 0]))
        Fin Pour
    Fin Pour
Fin

TDOL
Objet : Type/Nature
i, j : Entier
Convertir : Fonction

Fonction Nb_Occurrence (M : Mat, lig, col, C : Entier) : Entier
Début
    cpt <- 0
    Pour k de 1 à lig Faire
        Pour l de 1 à C Faire
            Si (k < lig) OU ((k = lig) ET (l < col)) Alors
                Si M[k, l] = M[lig, col] Alors
                    cpt <- cpt + 1
                Fin Si
            Fin Si
        Fin Pour
    Fin Pour
    Retourner cpt
Fin

TDOL
Objet : Type/Nature
k, l, cpt : Entier

Procédure Rectifier (CH : Chaine, @M : Mat, C : Entier)
Début
    Ouvrir(CH, f, "wb")
    Pour i de 1 à 8 Faire
        Pour j de 1 à C Faire
            nb <- Nb_Occurrence(M, i, j, C)
            Si nb > 0 Alors
                M[i, j] <- M[i, j] + "_" + ConvCh(nb)
            Fin Si
            e.ID <- M[0, j] + M[i, 0]
            e.CDA <- M[i, j]
            Ecrire(f, e)
        Fin Pour
    Fin Pour
    Fermer(f)
Fin

TDOL
Objet : Type/Nature
f : Fiche
e : Element
i, j, nb : Entier
Nb_Occurrence : Fonction

Algorithme Digicode
Début
    Saisir_C(C)
    CH <- "codes_acces.dat"
    Entete(M, C)
    Contenu(M, C)
    Rectifier(CH, M, C)
    Ecrire_NL("Fichier ", CH, " généré avec succès")
Fin

TDNT
Mat = Tableau [0..8, 0..100] de Chaine
Element = Enregistrement
    ID : Chaine
    CDA : Chaine
Fin
Fiche = Fichier de Element

TDO
Objet : Type/Nature
M : Mat
C : Entier
CH : Chaine
Saisir_C : Procédure
Entete : Procédure
Contenu : Procédure
Rectifier : Procédure
