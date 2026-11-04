const fs = require('fs');

fs.mkdirSync('tests/devoir_ben_arous', { recursive: true });

// Exercice 1 : 01_suite_motifs_escalier
const ex1_alg = `Procédure Saisie (@c : Caractère, @n : Entier)
Début
    Ecrire("Donner un caractère : ")
    Lire(c)
    Répéter
        Ecrire("Donner une limite strictement positive (n >= 1) : ")
        Lire(n)
    Jusqu'à (n >= 1)
Fin

Fonction inconnue (c : Caractère, n : Entier) : Chaine
Début
    Si n = 1 Alors
        Retourner c + c
    Sinon
        Retourner CHR(ORD(c) + n - 1) + inconnue(c, n - 1) + CHR(ORD(c) + n - 1)
    Fin Si
Fin

Procédure Afficher_Suite (c : Caractère, n : Entier)
Début
    Pour i de 1 à n Faire
        Ecrire_NL(inconnue(c, i))
    Fin Pour
Fin

TDOL
Objet : Type/Nature
i : Entier
inconnue : Fonction

Algorithme Exercice1
Début
    Saisie(c, n)
    Afficher_Suite(c, n)
Fin

TDO
Objet : Type/Nature
c : Caractère
n : Entier
Saisie : Procédure
Afficher_Suite : Procédure
`;

fs.writeFileSync('tests/devoir_ben_arous/01_suite_motifs_escalier.alg', ex1_alg);
fs.writeFileSync('tests/devoir_ben_arous/01_suite_motifs_escalier.in', 'A\r\n4\r\n');
fs.writeFileSync('tests/devoir_ben_arous/01_suite_motifs_escalier.out', 'Donner un caractère : \r\nDonner une limite strictement positive (n >= 1) : \r\nAA\r\nBAAB\r\nCBAABC\r\nDCBAABCD\r\n');

// Exercice 2 : 02_competition_bowling
// For 5 players: N = 5
// Player 1: A12345678B, rolls: 10 10 10 10 10 10 10 10 (mean 10.0)
// Player 2: C11112222D, rolls: 5 5 5 5 5 5 5 5 (mean 5.0)
// Player 3: E33334444F, rolls: 8 8 8 8 8 8 8 8 (mean 8.0)
// Player 4: G55556666H, rolls: 6 6 6 6 6 6 6 6 (mean 6.0)
// Player 5: I77778888J, rolls: 7 7 7 7 7 7 7 7 (mean 7.0)
const ex2_alg = `Procédure Saisir_N (@N : Entier)
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
    S <- 0
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
`;

fs.writeFileSync('tests/devoir_ben_arous/02_competition_bowling.alg', ex2_alg);

// Exercice 3 : 03_calcul_pi
const ex3_alg = `Procédure Saisir_Epsilon (@eps : Réel)
Début
    Répéter
        Ecrire("Donner la précision epsilon (ex: 0.0001) : ")
        Lire(eps)
    Jusqu'à (eps > 0) ET (eps < 1)
Fin

Fonction Prod (x : Entier) : Entier
Début
    Si x <= 2 Alors
        Retourner x
    Sinon
        Retourner x * Prod(x - 2)
    Fin Si
Fin

Fonction puis (x, y : Entier) : Entier
Début
    p <- 1
    Pour j de 1 à y Faire
        p <- p * x
    Fin Pour
    Retourner p
Fin

TDOL
Objet : Type/Nature
j, p : Entier

Fonction CalculPi (epsilon : Réel) : Réel
Début
    i <- 1
    s <- 1 / 2
    Répéter
        sp <- s
        terme <- (Prod(i) / Prod(i + 1)) * (1 / ((i + 2) * puis(2, i + 2)))
        s <- sp + terme
        i <- i + 2
    Jusqu'à Abs(s - sp) <= epsilon
    Retourner 6 * s
Fin

TDOL
Objet : Type/Nature
i : Entier
s, sp, terme : Réel
Prod : Fonction
puis : Fonction

Algorithme Approximation_Pi
Début
    Saisir_Epsilon(epsilon)
    pi_approx <- CalculPi(epsilon)
    Ecrire_NL("Valeur approchée de Pi : ", pi_approx)
Fin

TDO
Objet : Type/Nature
epsilon : Réel
pi_approx : Réel
Saisir_Epsilon : Procédure
CalculPi : Fonction
`;

fs.writeFileSync('tests/devoir_ben_arous/03_calcul_pi.alg', ex3_alg);

// Exercice 4 : 04_suite_racines_simplifiees
const ex4_alg = `Procédure Saisie_Param (@x, @K : Entier)
Début
    Répéter
        Ecrire("Donner x (x > 1) : ")
        Lire(x)
    Jusqu'à (x > 1)

    Répéter
        Ecrire("Donner le nombre de termes K (K >= 1) : ")
        Lire(K)
    Jusqu'à (K >= 1)
Fin

Fonction Terme (x, n : Entier) : Entier
Début
    Si n = 0 Alors
        Retourner x
    Sinon
        Retourner 3 * Terme(x, n - 1) - 2
    Fin Si
Fin

Procédure Remplir (@F : Fichier texte, K, x : Entier)
Début
    Ouvrir("source.txt", F, "w")
    Pour i de 0 à K - 1 Faire
        Ecrire_NL(F, ConvCh(Terme(x, i)))
    Fin Pour
    Fermer(F)
Fin

TDOL
Objet : Type/Nature
i : Entier
Terme : Fonction

Fonction Puissance (x, y : Entier) : Entier
Début
    P <- 1
    Pour i de 1 à y Faire
        P <- P * x
    Fin Pour
    Retourner P
Fin

TDOL
Objet : Type/Nature
i, P : Entier

Procédure Facteurs (n : Entier, @T : Tab, @nb : Entier)
Début
    i <- 2
    nb <- 0
    Répéter
        Si n Mod i = 0 Alors
            p <- 0
            Tant Que n Mod i = 0 Faire
                p <- p + 1
                n <- n Div i
            Fin Tant Que
            T[nb].fact <- i
            T[nb].puis <- p
            nb <- nb + 1
        Sinon
            i <- i + 1
        Fin Si
    Jusqu'à (n = 1)
Fin

TDOL
Objet : Type/Nature
i, p : Entier

Procédure Determiner (n : Entier, @a, @b : Entier)
Début
    Facteurs(n, T, nb)
    a <- 1
    b <- 1
    Pour i de 0 à nb - 1 Faire
        a <- a * Puissance(T[i].fact, T[i].puis Div 2)
        Si T[i].puis Mod 2 <> 0 Alors
            b <- b * T[i].fact
        Fin Si
    Fin Pour
Fin

TDOL
Objet : Type/Nature
T : Tab
nb, i : Entier
Facteurs : Procédure
Puissance : Fonction

Procédure Simplifier (@F : Fichier texte)
Début
    Ouvrir("source.txt", F, "r")
    Tant Que Non Fin_Fichier(F) Faire
        Lire_Ligne(F, ch)
        n <- Valeur(ch)
        Determiner(n, a, b)
        Si (a <> 1) ET (b <> 1) Alors
            Ecrire_NL("Racine(", ch, ") = ", a, " * Racine(", b, ")")
        Sinon Si (a = 1) ET (b <> 1) Alors
            Ecrire_NL("Racine(", ch, ") = Racine(", b, ")")
        Sinon Si (b = 1) Alors
            Ecrire_NL("Racine(", ch, ") = ", a)
        Fin Si
    Fin Tant Que
    Fermer(F)
Fin

TDOL
Objet : Type/Nature
ch : Chaine
a, b, n : Entier
Determiner : Procédure

Algorithme Racines_Suite
Début
    Saisie_Param(x, K)
    Remplir(F, K, x)
    Simplifier(F)
Fin

TDNT
facteur = Enregistrement
    fact : Entier
    puis : Entier
Fin
Tab = Tableau de 100 facteur

TDO
Objet : Type/Nature
F : Fichier texte
x, K : Entier
Saisie_Param : Procédure
Remplir : Procédure
Simplifier : Procédure
`;

fs.writeFileSync('tests/devoir_ben_arous/04_suite_racines_simplifiees.alg', ex4_alg);

// Exercice 5 : 05_matrice_digicodes
const ex5_alg = `Procédure Saisir_C (@C : Entier)
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
`;

fs.writeFileSync('tests/devoir_ben_arous/05_matrice_digicodes.alg', ex5_alg);

console.log('Test .alg files generated.');
