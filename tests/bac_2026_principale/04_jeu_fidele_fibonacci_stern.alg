Procédure TrianglePascal (@M : Mat, N : Entier)
Début
    Pour i de 0 à N - 1 Faire
        M[i, 0] <- 1
        M[i, i] <- 1
        Pour j de 1 à i - 1 Faire
            M[i, j] <- M[i - 1, j] + M[i - 1, j - 1]
        Fin Pour
    Fin Pour
Fin

TDOL
Objet : Type/Nature
i, j : Entier

Procédure Remplir (M : Mat, N : Entier, @TFS : TabTFS)
Début
    Pour k de 1 à N Faire
        lig <- k - 1
        col <- 0
        somFib <- 0
        nbSte <- 0
        Tant Que lig >= col Faire
            somFib <- somFib + M[lig, col]
            Si M[lig, col] Mod 2 <> 0 Alors
                nbSte <- nbSte + 1
            Fin Si
            lig <- lig - 1
            col <- col + 1
        Fin Tant Que
        TFS[k - 1].Fib <- somFib
        TFS[k - 1].Ste <- nbSte
    Fin Pour
Fin

TDOL
Objet : Type/Nature
k, lig, col, somFib, nbSte : Entier

Fonction ExisteFib (val : Entier, TFS : TabTFS, N : Entier) : Booléen
Début
    trouve <- Faux
    i <- 0
    Tant Que (i < N) ET Non trouve Faire
        Si TFS[i].Fib = val Alors
            trouve <- Vrai
        Sinon
            i <- i + 1
        Fin Si
    Fin Tant Que
    Retourner trouve
Fin

TDOL
Objet : Type/Nature
trouve : Booléen
i : Entier

Fonction ExisteSte (val : Entier, TFS : TabTFS, N : Entier) : Booléen
Début
    trouve <- Faux
    i <- 0
    Tant Que (i < N) ET Non trouve Faire
        Si TFS[i].Ste = val Alors
            trouve <- Vrai
        Sinon
            i <- i + 1
        Fin Si
    Fin Tant Que
    Retourner trouve
Fin

TDOL
Objet : Type/Nature
trouve : Booléen
i : Entier

Fonction SommeChiffres (mat : Chaine) : Entier
Début
    s <- 0
    Pour i de 0 à Long(mat) - 1 Faire
        s <- s + Valeur(mat[i])
    Fin Pour
    Retourner s
Fin

TDOL
Objet : Type/Nature
s, i : Entier

Algorithme Exercice4_Pascal_Fib_Stern
Début
    N <- 9

    // Création du fichier source Fidele.txt (exemple de l'énoncé page 5)
    Ouvrir("Fidele.txt", F_fid_init, "w")
    Ecrire_NL(F_fid_init, "4231")
    Ecrire_NL(F_fid_init, "2001")
    Ecrire_NL(F_fid_init, "1090")
    Ecrire_NL(F_fid_init, "0123")
    Ecrire_NL(F_fid_init, "0130")
    Ecrire_NL(F_fid_init, "7789")
    Ecrire_NL(F_fid_init, "9106")
    Ecrire_NL(F_fid_init, "3274")
    Ecrire_NL(F_fid_init, "3110")
    Fermer(F_fid_init)

    // Traitement
    TrianglePascal(M, N)
    Remplir(M, N, TFS)

    Ouvrir("Fidele.txt", F_fid, "r")
    Ouvrir("Gagnants.txt", F_gagn, "w")

    Tant Que Non Fin_Fichier(F_fid) Faire
        Lire_Ligne(F_fid, matricule)
        S <- SommeChiffres(matricule)
        isFib <- ExisteFib(S, TFS, N)
        isSte <- ExisteSte(S, TFS, N)

        Si isFib ET isSte Alors
            Ecrire_NL(F_gagn, matricule, " Super gagnant")
        Sinon Si isFib OU isSte Alors
            Ecrire_NL(F_gagn, matricule, " Gagnant")
        Fin Si
    Fin Tant Que

    Fermer(F_fid)
    Fermer(F_gagn)

    // Affichage des gagnants obtenus
    Ouvrir("Gagnants.txt", F_res, "r")
    Tant Que Non Fin_Fichier(F_res) Faire
        Lire_Ligne(F_res, ligne)
        Ecrire_NL(ligne)
    Fin Tant Que
    Fermer(F_res)
Fin

TDNT
Mat = Tableau [0..100, 0..100] de Entier
ElementTFS = Enregistrement
    Fib : Entier
    Ste : Entier
Fin
TabTFS = Tableau de 100 ElementTFS

TDO
Objet : Type/Nature
N, S : Entier
matricule, ligne : Chaine
isFib, isSte : Booléen
M : Mat
TFS : TabTFS
F_fid_init, F_fid, F_gagn, F_res : Fichier texte
TrianglePascal : Procédure
Remplir : Procédure
ExisteFib : Fonction
ExisteSte : Fonction
SommeChiffres : Fonction
