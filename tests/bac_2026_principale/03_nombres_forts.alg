Fonction SommeFacteurs (K : Entier) : Entier
Début
    i <- 2
    somme <- 0
    Tant Que K > 1 Faire
        Si K Mod i = 0 Alors
            somme <- somme + i
            Tant Que K Mod i = 0 Faire
                K <- K Div i
            Fin Tant Que
        Fin Si
        i <- i + 1
    Fin Tant Que
    Retourner somme
Fin

TDOL
Objet : Type/Nature
i, somme : Entier

Fonction EstFort (X, b : Entier) : Booléen
Début
    temp <- X
    somme_chiffres <- 0
    Tant Que temp > 0 Faire
        r <- temp Mod b
        somme_chiffres <- somme_chiffres + r
        temp <- temp Div b
    Fin Tant Que
    Retourner somme_chiffres = b
Fin

TDOL
Objet : Type/Nature
temp, somme_chiffres, r : Entier

Procédure NbreFort (N : Entier)
Début
    Ouvrir("NF.dat", f, "wb")
    Pour X de 10 à N Faire
        b <- SommeFacteurs(X)
        Si (b >= 2) ET (b <= 16) Alors
            Si EstFort(X, b) Alors
                e.Nbre <- X
                e.Base <- b
                Ecrire(f, e)
            Fin Si
        Fin Si
    Fin Pour
    Fermer(f)
Fin

TDOL
Objet : Type/Nature
f : Fichier de EnregFort
e : EnregFort
X, b : Entier
SommeFacteurs : Fonction
EstFort : Fonction

Algorithme Exercice3_NombresForts
Début
    N <- 230
    NbreFort(N)

    // Lecture et affichage des nombres forts trouvés
    Ouvrir("NF.dat", f_lec, "rb")
    Tant Que Non Fin_Fichier(f_lec) Faire
        Lire(f_lec, rec)
        Ecrire_NL("Nombre fort : ", rec.Nbre, " en base ", rec.Base)
    Fin Tant Que
    Fermer(f_lec)
Fin

TDNT
EnregFort = Enregistrement
    Nbre : Entier
    Base : Entier
Fin

TDO
Objet : Type/Nature
N : Entier
f_lec : Fichier de EnregFort
rec : EnregFort
NbreFort : Procédure
