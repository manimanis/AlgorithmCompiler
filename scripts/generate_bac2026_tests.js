const fs = require('fs');
const { execSync } = require('child_process');

fs.mkdirSync('tests/bac_2026_principale', { recursive: true });

// ==========================================
// EXERCICE 1 : Fonction Inconnue & Procédure Remplir
// ==========================================
const ex1_alg = `Fonction Inconnue (a, b : Entier) : Entier
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
`;

fs.writeFileSync('tests/bac_2026_principale/01_inconnue_et_remplir.alg', ex1_alg);
fs.writeFileSync('tests/bac_2026_principale/01_inconnue_et_remplir.in', '');

// ==========================================
// EXERCICE 2 : Fichier Compagnies & Tri Décroissant
// ==========================================
const ex2_alg = `Procédure Tri (Ch1, Ch2 : Chaine)
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
`;

fs.writeFileSync('tests/bac_2026_principale/02_tri_compagnies.alg', ex2_alg);
fs.writeFileSync('tests/bac_2026_principale/02_tri_compagnies.in', '');

// ==========================================
// EXERCICE 3 : Nombres Forts
// ==========================================
const ex3_alg = `Fonction SommeFacteurs (K : Entier) : Entier
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
`;

fs.writeFileSync('tests/bac_2026_principale/03_nombres_forts.alg', ex3_alg);
fs.writeFileSync('tests/bac_2026_principale/03_nombres_forts.in', '');

// ==========================================
// EXERCICE 4 : Triangle de Pascal, Fibonacci, Stern & Jeu Promo
// ==========================================
const ex4_alg = `Procédure TrianglePascal (@M : Mat, N : Entier)
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
`;

fs.writeFileSync('tests/bac_2026_principale/04_jeu_fidele_fibonacci_stern.alg', ex4_alg);
fs.writeFileSync('tests/bac_2026_principale/04_jeu_fidele_fibonacci_stern.in', '');

console.log('Generated all Bac 2026 tests.');
