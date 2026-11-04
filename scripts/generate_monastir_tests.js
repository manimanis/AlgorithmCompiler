const fs = require('fs');
const { execSync } = require('child_process');

fs.mkdirSync('tests/bac_blanc_monastir', { recursive: true });

// ==========================================
// EXERCICE 1 : Nombres Harmoniques
// ==========================================
const ex1_alg = `Procédure Saisie (@Vi, @Vf, @P : Entier)
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
`;

fs.writeFileSync('tests/bac_blanc_monastir/01_nombres_harmoniques.alg', ex1_alg);
const ex1_in = '2\n20\n2\n';
fs.writeFileSync('tests/bac_blanc_monastir/01_nombres_harmoniques.in', ex1_in);

// ==========================================
// EXERCICE 2 : Suite de Recamán, Gamme Musicale & Tri Shell
// ==========================================
const ex2_alg = `Procédure Init_Gamme (@G : TAB2)
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
`;

fs.writeFileSync('tests/bac_blanc_monastir/02_musique_recaman.alg', ex2_alg);
const ex2_in = '6\n';
fs.writeFileSync('tests/bac_blanc_monastir/02_musique_recaman.in', ex2_in);

// ==========================================
// EXERCICE 3 : Matrice de Seidel & Approximation de Pi
// ==========================================
const ex3_alg = `Fonction ApproximationPi (N : Entier) : Réel
Début
    E <- 1
    M[0, 0] <- 1
    Pour i de 1 à N - 1 Faire
        Ep <- E
        Si i Mod 2 <> 0 Alors
            M[i, 0] <- 0
            Pour j de 1 à i Faire
                M[i, j] <- M[i, j - 1] + M[i - 1, j - 1]
            Fin Pour
            E <- M[i, i]
        Sinon
            M[i, i] <- 0
            Pour j de i - 1 à 0 [Pas = -1] Faire
                M[i, j] <- M[i, j + 1] + M[i - 1, j]
            Fin Pour
            E <- M[i, 0]
        Fin Si
    Fin Pour
    Retourner (2 * i * Ep) / E
Fin

TDOL
Objet : Type/Nature
M : Matrice_Seidel
E, Ep, i, j : Entier

Algorithme Exercice3_Seidel
Début
    Répéter
        Ecrire("Donner le rang N pour la matrice de Seidel (ex: 9) : ")
        Lire(N)
    Jusqu'à (N >= 2) ET (N <= 20)
    pi_val <- ApproximationPi(N)
    Ecrire_NL("Valeur approchée de Pi : ", pi_val)
Fin

TDNT
Matrice_Seidel = Tableau [0..20, 0..20] de Entier

TDO
Objet : Type/Nature
N : Entier
pi_val : Réel
ApproximationPi : Fonction
`;

fs.writeFileSync('tests/bac_blanc_monastir/03_matrice_seidel.alg', ex3_alg);
const ex3_in = '9\n';
fs.writeFileSync('tests/bac_blanc_monastir/03_matrice_seidel.in', ex3_in);

// ==========================================
// EXERCICE 4 : Spirale de Cornu, Point Fixe & Aire des Trapèzes
// ==========================================
const ex4_alg = `Fonction ApproximationPi (N : Entier) : Réel
Début
    E <- 1
    M[0, 0] <- 1
    Pour i de 1 à N - 1 Faire
        Ep <- E
        Si i Mod 2 <> 0 Alors
            M[i, 0] <- 0
            Pour j de 1 à i Faire
                M[i, j] <- M[i, j - 1] + M[i - 1, j - 1]
            Fin Pour
            E <- M[i, i]
        Sinon
            M[i, i] <- 0
            Pour j de i - 1 à 0 [Pas = -1] Faire
                M[i, j] <- M[i, j + 1] + M[i - 1, j]
            Fin Pour
            E <- M[i, 0]
        Fin Si
    Fin Pour
    Retourner (2 * i * Ep) / E
Fin

TDOL
Objet : Type/Nature
M : Matrice_Seidel
E, Ep, i, j : Entier

Fonction PI (n : Entier) : Réel
Début
    Retourner ApproximationPi(n)
Fin

Fonction sin (X : Réel) : Réel
Début
    P2 <- X
    N <- 1
    pi_val <- PI(9)
    Répéter
        P1 <- P2
        P2 <- P2 * (1 - ((X * X) / (N * N * pi_val * pi_val)))
        N <- N + 1
    Jusqu'à Abs(P2 - P1) <= 0.000001
    Retourner P2
Fin

TDOL
Objet : Type/Nature
P1, P2, pi_val : Réel
N : Entier
PI : Fonction

Fonction f (X : Réel) : Réel
Début
    Retourner sin((PI(9) * X * X) / 2)
Fin

TDOL
Objet : Type/Nature
sin, PI : Fonction

Procédure Point_Raccordement (@x1, @x2 : Réel, eps : Réel)
Début
    x1 <- 0.2
    Tant Que Abs(f(x1) - x1) > eps Faire
        x1 <- x1 + eps
    Fin Tant Que

    x2 <- 1.2
    Tant Que Abs(f(x2) - x2) > eps Faire
        x2 <- x2 - eps
    Fin Tant Que
Fin

TDOL
Objet : Type/Nature
f : Fonction

Fonction Somme (x1, H : Réel, i : Entier) : Réel
Début
    Si i = 1 Alors
        Retourner f(x1 + H)
    Sinon
        Retourner f(x1 + i * H) + Somme(x1, H, i - 1)
    Fin Si
Fin

TDOL
Objet : Type/Nature
f : Fonction

Procédure Aire (x1, x2 : Réel, N : Entier)
Début
    H <- (x2 - x1) / N
    resultat <- (H / 2) * (f(x1) + f(x2) + 2 * Somme(x1, H, N - 1))
    Ecrire_NL("L'aire sous la courbe entre x1 et x2 est : ", resultat)
Fin

TDOL
Objet : Type/Nature
H, resultat : Réel
f, Somme : Fonction

Algorithme Clothoide_Ferroviaire
Début
    eps <- 0.0001
    Point_Raccordement(x1, x2, eps)
    Ecrire_NL("Point fixe x1 = ", x1)
    Ecrire_NL("Point fixe x2 = ", x2)
    Répéter
        Ecrire("Donner le nombre de trapèzes N (N >= 10) : ")
        Lire(N)
    Jusqu'à (N >= 10)
    Aire(x1, x2, N)
Fin

TDNT
Matrice_Seidel = Tableau [0..20, 0..20] de Entier

TDO
Objet : Type/Nature
x1, x2, eps : Réel
N : Entier
Point_Raccordement : Procédure
Aire : Procédure
`;

fs.writeFileSync('tests/bac_blanc_monastir/04_spirale_cornu.alg', ex4_alg);
const ex4_in = '10\n';
fs.writeFileSync('tests/bac_blanc_monastir/04_spirale_cornu.in', ex4_in);

// ==========================================
// EXERCICE 5 : QR Code Poste Tunisienne & Fichier Binaire
// ==========================================
const ex5_alg = `Procédure Init_Table (@T : TAB1)
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
`;

fs.writeFileSync('tests/bac_blanc_monastir/05_qrcode_poste.alg', ex5_alg);
fs.writeFileSync('tests/bac_blanc_monastir/05_qrcode_poste.in', '');

console.log('Generated all 5 Monastir test algorithms and input files.');
