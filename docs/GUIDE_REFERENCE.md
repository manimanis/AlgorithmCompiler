# 📖 Manuel de Référence Complet & Guide d'Utilisation

**AlgorithmCompiler v1.7.0**  
*Compilateur C++20, Interpréteur, Machine Virtuelle à Bytecode, Sérialiseur Binaire, Générateur d'Exécutables Autonomes (.exe), Extracteur de Signatures & Tables TDO/TDNT et Transpilateur Python pour le Pseudo-code Algorithmique du Baccalauréat Tunisien.*

---

## 📑 Table des Matières

1. [Introduction](#1-introduction)
2. [Interface Ligne de Commande (CLI `algo.exe`)](#2-interface-ligne-de-commande-cli-algoexe)
   - [Syntaxe générale](#syntaxe-générale)
   - [Options du compilateur](#options-du-compilateur)
   - [Codes de sortie (Exit Codes)](#codes-de-sortie-exit-codes)
   - [Exemples d'utilisation courante](#exemples-dutilisation-courante)
3. [Structure d'un Algorithme](#3-structure-dun-algorithme)
   - [Entête et Squelette](#entête-et-squelette)
   - [Table des Objets Globaux (TDO / TDOG)](#table-des-objets-globaux-tdo--tdog)
   - [Table des Nouveaux Types (TDNT)](#table-des-nouveaux-types-tdnt)
   - [Table des Objets Locaux (TDOL)](#table-des-objets-locaux-tdol)
   - [Section Constantes](#section-constantes)
   - [Importation de Modules (`Importer`)](#importation-de-modules-importer)
4. [Types de Données et Littéraux](#4-types-de-données-et-littéraux)
   - [Types primitifs](#types-primitifs)
   - [Notation Scientifique](#notation-scientifique)
   - [Types Énumérés & Sous-intervalles](#types-énumérés--sous-intervalles)
   - [Tableaux 1D](#tableaux-1d)
   - [Matrices 2D](#matrices-2d)
   - [Enregistrements (Records) et Structures Imbriquées](#enregistrements-records-et-structures-imbriquées)
   - [Fichiers Textes & Fichiers Typés](#fichiers-textes--fichiers-typés)
   - [Pointeurs et Structures Dynamiques](#pointeurs-et-structures-dynamiques)
5. [Opérateurs et Expressions](#5-opérateurs-et-expressions)
   - [Opérateurs Arithmétiques](#opérateurs-arithmétiques)
   - [Opérateurs de Comparaison / Relationnels](#opérateurs-de-comparaison--relationnels)
   - [Opérateurs Logiques](#opérateurs-logiques)
   - [Opérateur d'Affectation](#opérateur-daffectation)
   - [Opérateurs d'Appartenance et d'Intervalle](#opérateurs-dappartenance-et-dintervalle)
   - [Opérateur de Déréférencement](#opérateur-de-déréférencement)
6. [Toutes les Instructions Supportées](#6-toutes-les-instructions-supportées)
   - [6.1 Affectation](#61-affectation)
   - [6.2 Entrées / Sorties Console (`Ecrire`, `Lire`)](#62-entrées--sorties-console-ecrire-lire)
   - [6.3 Structure Conditionnelle (`Si ... Alors ... Sinon`)](#63-structure-conditionnelle-si--alors--sinon)
   - [6.4 Aiguillage Multiple (`Selon ... Cas`)](#64-aiguillage-multiple-selon--cas)
   - [6.5 Boucle Pour (`Pour ... de ... à`)](#65-boucle-pour-pour--de--à)
   - [6.6 Boucle Tant Que (`Tant Que ... Faire`)](#66-boucle-tant-que-tant-que--faire)
   - [6.7 Boucle Répéter (`Répéter ... Jusqu'à`)](#67-boucle-répéter-répéter--jusquà)
   - [6.8 Sous-programmes (`Procédure`, `Fonction`, `Retourner`)](#68-sous-programmes-procédure-fonction-retourner)
   - [6.9 Manipulation de Fichiers (`Ouvrir`, `Fermer`, `Lire_Ligne`, `Aller_A`)](#69-manipulation-de-fichiers-ouvrir-fermer-lire_ligne-aller_a)
   - [6.10 Gestion Dynamique de Mémoire (`Allouer`, `Liberer`)](#610-gestion-dynamique-de-mémoire-allouer-liberer)
7. [Bibliothèque Standard (Fonctions Prédéfinies)](#7-bibliothèque-standard-fonctions-prédéfinies)
   - [Fonctions Mathématiques](#fonctions-mathématiques)
   - [Fonctions de Manipulation de Chaînes & Caractères](#fonctions-de-manipulation-de-chaînes--caractères)
   - [Fonctions Fichiers](#fonctions-fichiers)
   - [Génération Aléatoire](#génération-aléatoire)
8. [Analyseur Sémantique & Messages d'Erreur](#8-analyseur-sémantique--messages-derreur)
9. [Transpilateur Python 3 (`--to-python`)](#9-transpilateur-python-3---to-python)
10. [Machine Virtuelle, Bytecode Binaire & Exécutables Autonomes (`--vm`, `--compile-bc`, `--build-exe`)](#10-machine-virtuelle-bytecode-binaire--exécutables-autonomes---vm---compile-bc---build-exe)
11. [Exemples de Structures Dynamiques](#11-exemples-de-structures-dynamiques)

---

## 1. Introduction

**AlgorithmCompiler** est un environnement d'exécution, d'analyse et de compilation moderne pour le pseudo-code algorithmique francophone officiel enseigné dans le système éducatif tunisien (Secondaire : 2ème, 3ème et 4ème année / Baccalauréat Scientifique, Technique, Économie et Informatique).

Développé en **C++20 moderne**, il réunit :
- Un **analyseur sémantique statique** offrant des messages d'erreur détaillés avec mise en évidence visuelle (`^~~~`) et suggestions intelligentes (Levenshtein).
- Un **interpréteur d'AST** haute précision reposant sur le patron Visiteur.
- Une **Machine Virtuelle à Bytecode (VM)** avec optimiseur peephole et fast-paths optimisés pour des performances d'exécution maximales.
- Un **transpilateur Python 3** produisant un code propre, idiomatique et sans dépendances superflues.
- Une suite complète de **230 tests unitaires** couvrant l'ensemble du programme officiel et les sujets récents du Baccalauréat.

---

## 2. Interface Ligne de Commande (CLI `algo.exe`)

### Syntaxe générale

```bash
algo.exe <fichier.alg> [OPTIONS]
algo.exe [OPTIONS]
```

### Options du compilateur

| Option | Description |
| :--- | :--- |
| `-h`, `--help` | Affiche l'aide détaillée de la ligne de commande et quitte. |
| `-v`, `--version` | Affiche les informations de version détaillée (SemVer, C++20, OS, GCC/Clang). |
| `--check` | Exécute uniquement l'analyse syntaxique et sémantique statique sans exécuter le code. |
| `--vm` | Exécute le programme via la **Machine Virtuelle à Bytecode** (Mode par défaut ultra-rapide). |
| `--ast` | Force l'exécution via le moteur d'interprétation d'AST (Patron Visiteur). |
| `--build-exe`, `-b [out.exe]` | Compile l'algorithme (`.alg` ou `.algb`) en un **exécutable autonome indépendant (.exe)** embarquant le bytecode et la VM. |
| `--compile-bc`, `-c [out.algb]` | Compile l'algorithme et sérialise le bytecode dans un fichier binaire compact `.algb`. |
| `-s`, `--signatures`, `--tdo [out]` | Renvoie la signature de tous les sous-programmes et leurs tables TDNT et TDO/TDOL respectifs (génère automatiquement le TDO/TDOL s'il n'existe pas). |
| `--json` | Formate la sortie en JSON (utilisable avec `--signatures`). |
| `--compare` | Exécute le programme d'abord avec l'AST puis avec la VM et compare les temps d'exécution. |
| `--time`, `--bench` | Mesure et affiche le temps d'exécution en millisecondes (`Temps d'exécution : X ms`). |
| `--dump-bytecode` | Désassemble et affiche les instructions bytecode du programme compilé sans l'exécuter. |
| `--to-python`, `--py [out.py]` | Transpile l'algorithme vers Python 3. Écrit dans `out.py` si spécifié, sinon stdout. |
| `--max-iter <N>` | Définit le nombre maximal d'itérations autorisées dans une boucle (Défaut : `1 000 000`). |
| `--timeout <ms>` | Définit le délai maximal d'exécution en millisecondes (Défaut : `50 000 ms`). |
| `--seed <N>` | Définit la graine initiale pseudo-aléatoire pour reproductibilité des tests (`Alea`). |

### Codes de sortie (Exit Codes)

| Code | Signification |
| :---: | :--- |
| `0` | Exécution terminée avec succès / Vérification sans erreur. |
| `1` | Erreur d'analyse lexicale, syntaxique, sémantique ou erreur à l'exécution. |

### Exemples d'utilisation courante

```bash
# 1. Exécution standard via la VM (fichier source .alg ou bytecode .algb)
algo.exe programme.alg
algo.exe programme.algb

# 2. Extraire les signatures et générer les tables TDO / TDNT
algo.exe programme.alg --signatures
algo.exe programme.alg -s
algo.exe programme.alg -s --json

# 3. Compiler en exécutable autonome indépendant (.exe)
algo.exe programme.alg --build-exe mon_programme.exe
# ou syntaxe rapide :
algo.exe programme.alg -b
# Exécution directe sur toute machine Windows :
.\mon_programme.exe

# 4. Compiler en bytecode binaire (.algb)
algo.exe programme.alg --compile-bc mon_programme.algb

# 5. Vérification sémantique statique (mode pédagogique / CI)
algo.exe devoir.alg --check

# 5. Mesurer la performance d'exécution
algo.exe tri.alg --time

# 6. Examiner le bytecode généré
algo.exe calcul.alg --dump-bytecode

# 7. Transpiler vers un script Python autonome
algo.exe algo_bac.alg --to-python solution.py

# 8. Exécution sécurisée avec graine fixe et limite de boucles
algo.exe simulation.alg --seed 12345 --max-iter 500000
```

---

## 3. Structure d'un Algorithme

### Entête et Squelette

Un fichier algorithmique comporte typiquement le nom de l'algorithme, les sous-programmes, le bloc principal et les tables de déclarations :

```algorithme
Algorithme Calcul_Moyenne
Début
    Ecrire("Calcul de la moyenne :")
    // Instructions principales
Fin

TDO
Objet : Type/Nature
note1, note2 : réel
moy : réel
```

> **Note :** La casse des mots-clés est insensible (`ALGORITHME`, `Algorithme` ou `algorithme` sont acceptés). Les accents français sont supportés (`Début`, `Debut`, `Procédure`, `Procedure`, `Fin Si`, `FinSi`, etc.).

### Table des Objets Globaux (TDO / TDOG)

Placée en fin de fichier (ou en début), elle déclare les variables et constantes globales :

```algorithme
TDO
Objet : Type/Nature
i, n : entier
nom : chaîne
tab : Tableau de 50 entier
PI : Constante = 3.14159
```

### Table des Nouveaux Types (TDNT)

Permet de définir des alias de types, des énumérations, des tableaux typés et des enregistrements personnalisés :

```algorithme
TDNT
TabNotes = Tableau de 30 réel
Mat10 = Tableau de 10 lignes * 10 colonnes de entier
Etudiant = Enregistrement
    matricule : entier
    nom : chaîne
    moyenne : réel
Fin
Jour = ("Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche")
NoteValide = 0 .. 20
```

### Table des Objets Locaux (TDOL)

Chaque sous-programme (`Procédure` ou `Fonction`) peut posséder son propre tableau local de variables et constantes :

```algorithme
Procédure Remplir(@t : TabNotes; n : entier)
Début
    Pour i de 0 à n - 1 Faire
        t[i] ← Alea(0, 20)
    Fin Pour
Fin

TDOL
Objet : Type/Nature
i : entier
```

### Section Constantes

Peut être déclarée dans un bloc dédié ou directement au sein d'un `TDO`/`TDOL` :

```algorithme
Const
    TAILLE_MAX = 100
    SEUIL = 10.5
    TITRE = "Session Principale"
```

### Importation de Modules (`Importer`)

Permet de réutiliser des bibliothèques de fonctions et de types contenues dans d'autres fichiers `.alg` :

```algorithme
Importer "modules/math_utils.alg"
Importer Racine, Puissance Depuis "modules/calculs.alg"
```

---

## 4. Types de Données et Littéraux

### Types primitifs

| Type | Description | Équivalent C++ / Python | Littéraux valides |
| :--- | :--- | :--- | :--- |
| `entier` | Entier signé 64-bit | `int64_t` / `int` | `0`, `42`, `-105`, `1000000` |
| `réel` / `reel` | Flottant double précision (64-bit IEEE 754) | `double` / `float` | `3.14`, `-0.5`, `.75`, `1e-6` |
| `caractère` / `caractere` | Caractère unique ou chaîne de longueur 1 | `char` / `str` | `'A'`, `'z'`, `'9'`, `'\n'` |
| `chaîne` / `chaine` | Chaîne de texte UTF-8 | `std::string` / `str` | `"Tunisie"`, `"Bac 2026"`, `""` |
| `booléen` / `booleen` | Valeur booléenne vraie ou fausse | `bool` / `bool` | `Vrai`, `Faux` (ou `True`, `False`) |

### Notation Scientifique

La notation exponentielle est pleinement reconnue dans l'analyseur lexical et la VM :
```algorithme
tau ← R * C * 1e-9
masse ← 5.972E+24
seuil ← 2.5e-3
```

### Types Énumérés & Sous-intervalles

```algorithme
TDNT
Couleur = (Rouge, Vert, Bleu, Jaune)
Chiffre = 0 .. 9
LettreMaj = "A" .. "Z"
```

### Tableaux 1D

Déclaration avec taille statique :
```algorithme
TDO
t : Tableau de 20 entier
notes : Tableau de 35 réel
```
- Indexation à base 0 ou 1 selon l'usage : `t[0] ← 12`, `x ← notes[i]`.
- Contrôle strict des bornes à l'exécution avec message clair en cas de dépassement.

### Matrices 2D

```algorithme
TDNT
Grille = Tableau de 5 lignes * 5 colonnes de entier
// Variante acceptée :
Tableau de 5, 5 de entier
```
- Accès par virgule ou par crochets multiples : `M[i, j]` ou `M[i][j]`.

### Enregistrements (Records) et Structures Imbriquées

```algorithme
TDNT
Date = Enregistrement
    jour : entier
    mois : entier
    annee : entier
Fin

Eleve = Enregistrement
    nom : chaîne
    naissance : Date
    notes : Tableau de 5 réel
Fin

TDO
e : Eleve
```
- Accès aux champs par le point : `e.nom ← "Amira"`, `e.naissance.annee ← 2007`, `e.notes[0] ← 18.5`.
- Copie globale directe : `e2 ← e1`.

### Fichiers Textes & Fichiers Typés

- **Fichier Texte** : Séquence de lignes textuelles manipulables avec `Ouvrir`, `Lire_Ligne`, `Ecrire`, `Fin_Fichier`, `Fermer`.
- **Fichier de Données Typé (`Fichier d'<Type>`)** : Stocke directement des enregistrements ou des valeurs binaires avec accès direct possible via `Aller_A` et `Position_Fichier`.

### 4.8 Pointeurs et Structures Dynamiques

Le langage supporte les pointeurs typés permettant l'allocation dynamique sur le tas (heap) et la création de structures de données récursives (listes chaînées simples/doubles, piles, files, arbres, graphes).

#### Syntaxe de Déclaration
- Syntaxe standard : `p : ^Type`
- Syntaxe explicite équivalente : `p : Pointeur sur Type`
- Valeur nulle : constante `NIL` (alias acceptés : `NULL`, `NUL`).

```algorithme
TDNT
Noeud = Enregistrement
    valeur : entier
    suivant : ^Noeud
Fin

TDO
tete : ^Noeud
p : Pointeur sur Noeud
val_ptr : ^entier
```

#### Caractéristiques sémantiques
- **Déréférencement explicite** : `p^` permet d'accéder à la valeur ou à la structure pointée.
- **Accès direct aux champs (Sucre syntaxique)** : `p.champ` est équivalent à `p^.champ`.
- **Partage et sémantique par référence** : Contrairement aux enregistrements statiques copiés par valeur, l'affectation `q ← p` copie l'adresse mémoire du tas. Toute modification effectuée via `q^` ou `q.champ` se répercute immédiatement sur `p^`.
- **Sécurité à l'exécution** : Toute tentative de déréférencement d'un pointeur `NIL` ou non initialisé déclenche une erreur à l'exécution explicite (`Déréférencement d'un pointeur NIL ou non initialisé`).

---

## 5. Opérateurs et Expressions

### Opérateurs Arithmétiques

| Opérateur | Symbole | Description | Exemple |
| :--- | :---: | :--- | :--- |
| Addition | `+` | Somme ou concaténation de chaînes | `a + b`, `"Bac " + "2026"` |
| Soustraction | `-` | Différence | `x - y` |
| Multiplication | `*` | Produit | `base * hauteur` |
| Division réelle | `/` | Quotient réel flottant | `7 / 2` $\rightarrow 3.5$ |
| Division entière | `div` | Quotient euclidien entier | `7 div 2` $\rightarrow 3$, `-5 div 3` $\rightarrow -2$ |
| Modulo | `mod` ou `%` | Reste de la division euclidienne | `7 mod 2` $\rightarrow 1$, `-5 mod 3` $\rightarrow 1$ |
| Puissance | `^` ou `**` | Élévation à la puissance | `2 ^ 10` $\rightarrow 1024$ |
| Moins unaire | `-` | Négation arithmétique | `-x` |

### Opérateurs de Comparaison / Relationnels

| Opérateur | Symboles équivalents | Description |
| :--- | :---: | :--- |
| Égal | `=` | Égalité de valeurs (nombres, chaînes, booléens) |
| Différent | `≠`, `!=`, `<>` | Non-égalité |
| Inférieur strict | `<` | Plus petit que |
| Inférieur ou égal | `≤`, `<=` | Plus petit ou égal |
| Supérieur strict | `>` | Plus grand que |
| Supérieur ou égal | `≥`, `>=` | Plus grand ou égal |

### Opérateurs Logiques

| Opérateur | Symbole | Description |
| :--- | :---: | :--- |
| Conjonction | `Et` | Vrai si les deux opérandes sont vrais |
| Disjonction | `Ou` | Vrai si au moins un opérande est vrai |
| Négation | `Non` | Inverse la valeur de vérité |

### Opérateur d'Affectation

L'affectation peut s'écrire avec la flèche officielle Unicode ou en notation ASCII :
- `←` (Flèche Unicode standard)
- `<-` (Flèche ASCII équivalente)
- `:=` (Syntaxe Pascal/Bac acceptée)

```algorithme
x ← 10
compteur <- compteur + 1
somme := somme + x
```

### Opérateurs d'Appartenance et d'Intervalle

- **Appartenance** : `∈` ou `dans` pour tester la présence dans un intervalle ou un ensemble :
  ```algorithme
  Si x ∈ [1 .. 100] Alors ...
  Si carac dans ["A" .. "Z"] Alors ...
  ```
- **Intervalle** : `..` (ex. `1 .. 10`).

### 5.6 Opérateur de Déréférencement (`^`)

L'opérateur postfixe `^` permet d'accéder au contenu de la case mémoire allouée et pointée par un pointeur :
- En lecture d'expression : `x ← p^` ou `x ← p^.valeur`.
- En modification (L-value) : `p^ ← 42` ou `p^.valeur ← 10`.
- Équivalence de commodité : `p.valeur` est automatiquement converti en `p^.valeur`.

---

## 6. Toutes les Instructions Supportées

### 6.1 Affectation

Permet de stocker le résultat d'une expression dans une variable, une case de tableau, une cellule de matrice ou un champ d'enregistrement.

```algorithme
// Affectation simple
total ← 0

// Affectation de tableau 1D
t[i] ← t[i] * 2

// Affectation de matrice 2D
mat[i, j] ← val

// Affectation de champ d'enregistrement
etudiant.moyenne ← (note1 + note2) / 2

// Affectation d'enregistrement imbriqué
client.adresse.code_postal ← 1000
```

---

### 6.2 Entrées / Sorties Console (`Ecrire`, `Lire`)

#### Instruction `Ecrire` / `Ecrire_NL`
Affiche une ou plusieurs expressions séparées par des virgules.
- Supporte les paramètres de formatage optionnels `sep = "..."` et `fin = "..."` (ou `end = "..."`).
- `Ecrire_NL(...)` garantit un retour à la ligne à la fin.

```algorithme
Ecrire("Bonjour tout le monde")
Ecrire("Valeur de x =", x, "et de y =", y)
Ecrire("Elements :", a, b, c, sep = " | ", fin = "\n")
Ecrire_NL("Fin du traitement.")
```

#### Instruction `Lire`
Lit une ou plusieurs valeurs saisies par l'utilisateur au clavier. Le compilateur convertit automatiquement la saisie vers le type de la variable cible (`entier`, `réel`, `chaîne`, `booléen`).

```algorithme
Ecrire("Entrez votre nom puis votre âge : ")
Lire(nom, age)
```

---

### 6.3 Structure Conditionnelle (`Si ... Alors ... Sinon`)

Exécute un bloc selon la valeur d'une expression booléenne. Supporte `Sinon Si` pour l'enchaînement.

```algorithme
Si score >= 16 Alors
    mention ← "Très Bien"
Sinon Si score >= 14 Alors
    mention ← "Bien"
Sinon Si score >= 12 Alors
    mention ← "Assez Bien"
Sinon Si score >= 10 Alors
    mention ← "Passable"
Sinon
    mention ← "Ajourné"
Fin Si
```

---

### 6.4 Aiguillage Multiple (`Selon ... Cas`)

Permet d'orienter le flux selon plusieurs valeurs ponctuelles, listes de valeurs ou intervalles.

```algorithme
Selon choix Faire
    Cas 1 :
        Ecrire("Création d'un compte")
    Cas 2, 3 :
        Ecrire("Consultation ou Modification")
    Cas 4 .. 9 :
        Ecrire("Options avancées")
    Autrement :
        Ecrire("Choix invalide !")
Fin Selon
```

---

### 6.5 Boucle Pour (`Pour ... de ... à`)

Boucle itérative avec compteur. Supporte :
- L'incrémentation automatique par défaut (`Pas 1`).
- La spécification explicite du pas (`Pas p`).
- Les pas positifs ou négatifs (décrémentation).

```algorithme
// Boucle croissante par défaut
Pour i de 0 à 9 Faire
    Ecrire(i)
Fin Pour

// Boucle avec pas explicite
Pour k de 10 à 0 Pas -2 Faire
    Ecrire("k =", k)
Fin Pour
```

---

### 6.6 Boucle Tant Que (`Tant Que ... Faire`)

Boucle à précondition. L'itération se répète tant que la condition reste vraie.

```algorithme
Tant Que (n > 1) Faire
    Si n mod 2 = 0 Alors
        n ← n div 2
    Sinon
        n ← 3 * n + 1
    Fin Si
Fin Tant Que
```

---

### 6.7 Boucle Répéter (`Répéter ... Jusqu'à`)

Boucle à postcondition. Le corps est exécuté au moins une fois, puis l'exécution s'arrête dès que la condition devient vraie.
- Supporte la notation de doubles inégalités courante au Baccalauréat tunisien (ex: `0 < n ≤ 100`).

```algorithme
Répéter
    Ecrire("Donner un entier pair n (2 .. 50) : ")
    Lire(n)
Jusqu'à (n >= 2) Et (n <= 50) Et (n mod 2 = 0)

// Variante avec double inégalité :
Répéter
    Lire(taille)
Jusqu'à 0 < taille ≤ 100
```

---

### 6.8 Sous-programmes (`Procédure`, `Fonction`, `Retourner`)

#### Définition d'une Procédure
Une procédure effectue des actions et peut modifier des arguments passés par référence (`@`).

```algorithme
Procédure Echanger(@a : entier; @b : entier)
Début
    temp ← a
    a ← b
    b ← temp
Fin

TDOL
Objet : Type/Nature
temp : entier
```

#### Définition d'une Fonction
Une fonction retourne obligatoirement une valeur calculée via l'instruction `Retourner`.

```algorithme
Fonction Est_Premier(n : entier) : booléen
Début
    Si n < 2 Alors
        Retourner Faux
    Fin Si
    Pour i de 2 à Ent(Racine(n)) Faire
        Si n mod i = 0 Alors
            Retourner Faux
        Fin Si
    Fin Pour
    Retourner Vrai
Fin

TDOL
Objet : Type/Nature
i : entier
```

---

### 6.9 Manipulation de Fichiers (`Ouvrir`, `Fermer`, `Lire_Ligne`, `Aller_A`)

| Instruction / Fonction | Description | Exemple |
| :--- | :--- | :--- |
| `Ouvrir(f, path, mode)` | Ouvre un fichier en mode `"r"`, `"w"`, `"a"`, `"rb"`, `"wb"` ou `"ab"`. | `Ouvrir(f, "data.txt", "w")` |
| `Fermer(f)` | Ferme le flux du fichier. | `Fermer(f)` |
| `Lire_Ligne(f, ch)` | Lit une ligne entière depuis un fichier texte dans `ch`. | `Lire_Ligne(f, ligne)` |
| `Fin_Fichier(f)` | Renvoie `Vrai` si la fin du fichier est atteinte. | `Tant Que Non Fin_Fichier(f) Faire` |
| `Aller_A(f, pos)` | Déplace la tête de lecture/écriture du fichier typé à l'enregistrement `pos`. | `Aller_A(f, 0)` |
| `Taille_Fichier(f)` | Renvoie le nombre total d'enregistrements du fichier. | `n ← Taille_Fichier(f)` |
| `Position_Fichier(f)` | Renvoie l'indice de l'enregistrement courant. | `p ← Position_Fichier(f)` |
| `Ecrire(f, val)` | Écrit une valeur ou un enregistrement dans le fichier typé. | `Ecrire(f, etudiant)` |
| `Lire(f, var)` | Lit le prochain enregistrement du fichier typé. | `Lire(f, etudiant)` |

Exemple de parcours d'un fichier texte :
```algorithme
Ouvrir(f, "notes.txt", "r")
Tant Que Non Fin_Fichier(f) Faire
    Lire_Ligne(f, ligne)
    Ecrire(ligne)
Fin Tant Que
Fermer(f)
```

---

### 6.10 Gestion Dynamique de Mémoire (`Allouer`, `Liberer`)

| Procédure | Description | Exemple |
| :--- | :--- | :--- |
| `Allouer(p)` | Alloue un nouveau bloc mémoire sur le tas selon le type de base pointé par `p` et stocke l'adresse résultante dans la variable `p`. | `Allouer(nouveau_noeud)` |
| `Liberer(p)` | Libère le bloc mémoire sur le tas pointé par `p` et réaffecte automatiquement `NIL` à `p`. | `Liberer(noeud)` |

Exemple :
```algorithme
Allouer(p)
p^.valeur ← 42
p^.suivant ← NIL
Ecrire("Valeur allouée :", p.valeur)
Liberer(p)
// p est désormais égal à NIL
```

---

## 7. Bibliothèque Standard (Fonctions Prédéfinies)

### Fonctions Mathématiques

| Fonction | Description | Type retour | Exemple |
| :--- | :--- | :---: | :--- |
| `Racine(x)` | Racine carrée ($\sqrt{x}$) | `réel` | `Racine(16)` $\rightarrow 4.0$ |
| `Carre(x)` | Carré ($x^2$) | `réel` / `entier` | `Carre(5)` $\rightarrow 25$ |
| `Abs(x)` | Valeur absolue ($\|x\|$) | même type | `Abs(-7.5)` $\rightarrow 7.5$ |
| `Exp(x)` | Exponentielle ($e^x$) | `réel` | `Exp(1)` $\rightarrow 2.71828...$ |
| `Ln(x)` ou `Log(x)` | Logarithme népérien | `réel` | `Ln(10)` $\rightarrow 2.30258...$ |
| `Ent(x)` | Partie entière (troncature de la partie décimale) | `entier` | `Ent(3.9)` $\rightarrow 3$, `Ent(-3.9)` $\rightarrow -3$ |
| `Arrondi(x)` | Arrondi à l'entier le plus proche | `entier` | `Arrondi(3.6)` $\rightarrow 4$ |
| `Frac(x)` | Partie fractionnaire ($x - \lfloor x \rfloor$) | `réel` | `Frac(3.14)` $\rightarrow 0.14$ |
| `Sin(x)`, `Cos(x)`, `Tan(x)` | Fonctions trigonométriques (radians) | `réel` | `Cos(0)` $\rightarrow 1.0$ |
| `Arctan(x)` ou `Atan(x)` | Arc tangente | `réel` | `Arctan(1)` $\rightarrow 0.78539...$ |
| `Puissance(b, e)` | Élévation à la puissance ($b^e$) | `réel` / `entier` | `Puissance(2, 8)` $\rightarrow 256$ |

### Fonctions de Manipulation de Chaînes & Caractères

| Fonction | Description | Exemple |
| :--- | :--- | :--- |
| `Long(ch)` | Longueur d'une chaîne UTF-8 (ou nombre d'éléments d'un tableau) | `Long("Tunisie")` $\rightarrow 7$ |
| `Chr(code)` | Caractère ASCII correspondant au code | `Chr(65)` $\rightarrow \text{'A'}$ |
| `Ord(c)` | Code ASCII d'un caractère ou valeur ordinale d'un énuméré | `Ord('A')` $\rightarrow 65$ |
| `ConvCh(val)` | Convertit un nombre en chaîne de caractères | `ConvCh(123)` $\rightarrow \text{"123"}$ |
| `Valeur(ch)` | Convertit une chaîne numérique en entier ou réel | `Valeur("45.6")` $\rightarrow 45.6$ |
| `Sous_Chaine(ch, deb, fin)` | Extrait la sous-chaîne entre les positions `deb` et `fin` | `Sous_Chaine("Algorithme", 0, 4)` $\rightarrow \text{"Algo"}$ |
| `Pos(sous_ch, ch)` | Indice de la première occurrence de `sous_ch` dans `ch` (-1 si absente) | `Pos("go", "Algorithme")` $\rightarrow 2$ |
| `Majus(ch)` | Convertit une chaîne en lettres majuscules | `Majus("bac")` $\rightarrow \text{"BAC"}$ |
| `Minus(ch)` | Convertit une chaîne en lettres minuscules | `Minus("BAC")` $\rightarrow \text{"bac"}$ |
| `Effacer(ch, deb, n)` | Supprime $n$ caractères à partir de la position $deb$ | `Effacer("ABCDEF", 1, 3)` $\rightarrow \text{"AEF"}$ |
| `Inserer(ch1, ch2, pos)` | Insère `ch1` dans `ch2` à la position `pos` | `Inserer("-", "AB", 1)` $\rightarrow \text{"A-B"}$ |
| `EstNum(ch)` / `Est_Num(ch)` | Vérifie si une chaîne représente un nombre valide | `EstNum("123.45")` $\rightarrow \text{Vrai}$ |

### Fonctions Fichiers

| Fonction | Description |
| :--- | :--- |
| `Fin_Fichier(f)` | Renvoie `Vrai` si la fin du fichier est atteinte. |
| `Taille_Fichier(f)` / `Taille(f)` | Nombre total d'enregistrements du fichier. |
| `Position_Fichier(f)` / `Pointeur(f)` | Position actuelle dans le fichier. |

### Génération Aléatoire

| Fonction | Description | Exemple |
| :--- | :--- | :--- |
| `Alea(min, max)` | Tire un entier pseudo-aléatoire uniforme dans l'intervalle $[min, max]$ | `x ← Alea(1, 6)` |
| `Graine(seed)` | Initialise la graine du générateur aléatoire | `Graine(42)` |

---

## 8. Analyseur Sémantique & Messages d'Erreur

L'analyseur sémantique (`algo.exe --check`) valide l'intégralité du code avant toute exécution et signale :
- Les variables, procédures ou fonctions non déclarées dans le TDO/TDOL.
- Les incohérences de types (affectation d'une chaîne dans un entier, condition non booléenne dans un `Si` ou `Tant Que`).
- Les violations d'arité (nombre d'arguments incorrect lors de l'appel).
- Les passages d'expressions constantes ou de littéraux à des paramètres passés par référence (`@param`).
- Les accès à des champs inexistants dans des enregistrements.
- Les indices non entiers ou bornes de boucles `Pour` non numériques.
- Les fonctions sans retour ou le code mort après une instruction `Retourner`.

#### Exemple de diagnostic visuel avec suggestion intelligente :
```text
Erreur sémantique (ligne 4, colonne 10) :
      Ecrire(totla + 1)
             ^~~~~
La variable 'totla' non définie (objet non déclaré dans le TDO/TDOL).
Suggestion : Vouliez-vous dire 'total' ? (ligne 4)
```

---

## 9. Transpilateur Python 3 (`--to-python`)

AlgorithmCompiler intègre un convertisseur vers **Python 3 moderne et idiomatique**.

### Correspondances clés du Transpilateur

| Pseudo-Code Algorithmique | Python 3 Généré |
| :--- | :--- |
| `t : Tableau de 10 entier` | `t = np.zeros(10, dtype=int)` |
| `m : Tableau de 3, 3 de réel` | `m = np.zeros((3, 3), dtype=float)` |
| `e : Etudiant` (Enregistrement) | `e = {'nom': "", 'age': 0, 'moyenne': 0.0}` |
| `p : ^Noeud` / `Allouer(p)` | `p = None` / `p = {'valeur': 0, 'suivant': None}` |
| `p.valeur` / `p^` | `p['valeur']` / `p[0]` |
| `Liberer(p)` | `p = None` |
| `NIL` / `NULL` | `None` |
| `Répéter ... Jusqu'à condition` | `condition = False` / `while not condition:` *(Pas de `while True: break`)* |
| `Pos(s, ch)` | `ch.find(s)` |
| `Majus(ch)` / `Minus(ch)` | `ch.upper()` / `ch.lower()` |
| `Ecrire(...)` | `print(...)` |
| `Lire(x)` | `x = int(input())` ou `float(input())` |

---

## 10. Machine Virtuelle, Bytecode Binaire & Exécutables Autonomes (`--vm`, `--compile-bc`, `--build-exe`)

La Machine Virtuelle à Bytecode d'AlgorithmCompiler offre des gains de performance de plus de **75x** par rapport à l'évaluation pure d'AST :
- **Fast-paths 1D pour tableaux** : Opcodes spécialisés (`OP_ARRAY_GET_GLOBAL`, `OP_ARRAY_SET_GLOBAL`, `OP_ARRAY_GET_LOCAL`, `OP_ARRAY_SET_LOCAL`) éliminant toute copie inutile.
- **Fast-paths arithmétiques** : Traitement direct inliné des opérandes entiers 64-bit pour `+`, `-`, `*`, `div`, `mod`, `<`.
- **Gestion des pointeurs et tas VM** : Opcodes spécialisés (`OP_ALLOC_PTR`, `OP_FREE_PTR`, `OP_DEREF`, `OP_REF_DEREF`) pour l'allocation sur le tas, le déréférencement sécurisé et la détection d'accès `NIL`.
- **Optimiseur Peephole** : Élimination des instructions inutiles et chaînage direct des sauts conditionnels (`OP_JUMP_IF_FALSE`).
- **Pliage de constantes (Constant Folding)** : Évaluation au moment de la compilation des sous-expressions statiques (ex. `2 * 3.14 * 10`).

### 10.1 Visualisation du Désassemblage
```bash
algo.exe mon_programme.alg --dump-bytecode
```

### 10.2 Bytecode Binaire Sérialisé (`.algb`)
AlgorithmCompiler permet de compiler un algorithme en un fichier binaire compact `.algb` avec le format standardisé (`ALGB` magic number) :
```bash
# Compilation vers bytecode binaire :
algo.exe mon_programme.alg --compile-bc mon_programme.algb

# Exécution directe du bytecode binaire :
algo.exe mon_programme.algb
```

### 10.3 Génération d'Exécutables Autonomes Indépendants (`.exe`)
Vous pouvez générer un exécutable `.exe` Windows 100% autonome et sans dépendances à partir de n'importe quel code source `.alg` ou fichier bytecode `.algb` :
```bash
algo.exe mon_programme.alg --build-exe mon_programme.exe
# ou syntaxe rapide :
algo.exe mon_programme.alg -b
```

**Fonctionnement technique :**
- L'exécutable généré utilise le patron d'overlay PE (`PE Overlay Pattern`) : le bytecode sérialisé est encapsulé directement à la fin du binaire avec un marqueur `ALGBEXEC` et sa taille en 64-bit.
- Aucune compilation C++ ni chaîne d'outils externe (GCC, MinGW, Clang) n'est requise lors de la génération.
- L'exécutable généré est statiquement lié (`-static -static-libgcc -static-libstdc++`) et ne dépend d'aucune DLL tierce. Il fonctionne directement sur n'importe quel PC Windows en double-cliquant dessus ou depuis une invite de commandes.

---

## 11. Exemples de Structures Dynamiques

### 11.1 Pile Dynamique (LIFO - Dernier Entré, Premier Sorti)

```algorithme
Algorithme TestPile
TDNT
Element = Enregistrement
    valeur : entier
    suivant : ^Element
Fin
Pile = ^Element

Procédure Empiler(@p : Pile; val : entier)
Début
    Allouer(nouveau)
    nouveau.valeur ← val
    nouveau.suivant ← p
    p ← nouveau
Fin
TDOL
nouveau : Pile

Procédure Depiler(@p : Pile; @val : entier)
Début
    Si p ≠ NIL Alors
        val ← p.valeur
        temp ← p
        p ← p.suivant
        Liberer(temp)
    Fin Si
Fin
TDOL
temp : Pile

Début
    sommet ← NIL
    Empiler(sommet, 10)
    Empiler(sommet, 20)
    Empiler(sommet, 30)

    Tant Que sommet ≠ NIL Faire
        Depiler(sommet, v)
        Ecrire("Dépilé :", v)
    Fin Tant Que
Fin
TDO
sommet : Pile
v : entier
```

### 11.2 File Dynamique (FIFO - Premier Entré, Premier Sorti)

```algorithme
Algorithme TestFile
TDNT
Noeud = Enregistrement
    donnee : entier
    suivant : ^Noeud
Fin
File = Enregistrement
    tete : ^Noeud
    queue : ^Noeud
Fin

Procédure Enfiler(@f : File; val : entier)
Début
    Allouer(nouveau)
    nouveau.donnee ← val
    nouveau.suivant ← NIL
    Si f.tete = NIL Alors
        f.tete ← nouveau
        f.queue ← nouveau
    Sinon
        f.queue.suivant ← nouveau
        f.queue ← nouveau
    Fin Si
Fin
TDOL
nouveau : ^Noeud

Début
    f.tete ← NIL
    f.queue ← NIL
    Enfiler(f, 100)
    Enfiler(f, 200)
    Ecrire("Tête :", f.tete.donnee, "Queue :", f.queue.donnee)
Fin
TDO
f : File
```

### 11.3 Arbre Binaire de Recherche (ABR)

```algorithme
Algorithme TestArbre
TDNT
NoeudArbre = Enregistrement
    valeur : entier
    gauche : ^NoeudArbre
    droite : ^NoeudArbre
Fin
Arbre = ^NoeudArbre

Procédure Inserer(@racine : Arbre; val : entier)
Début
    Si racine = NIL Alors
        Allouer(racine)
        racine.valeur ← val
        racine.gauche ← NIL
        racine.droite ← NIL
    Sinon Si val < racine.valeur Alors
        Inserer(racine.gauche, val)
    Sinon
        Inserer(racine.droite, val)
    Fin Si
Fin

Procédure AfficherInfixe(racine : Arbre)
Début
    Si racine ≠ NIL Alors
        AfficherInfixe(racine.gauche)
        Ecrire(racine.valeur)
        AfficherInfixe(racine.droite)
    Fin Si
Fin

Début
    racine ← NIL
    Inserer(racine, 50)
    Inserer(racine, 30)
    Inserer(racine, 70)
    Ecrire("Parcours Infixe :")
    AfficherInfixe(racine)
Fin
TDO
racine : Arbre
```

---

*Document généré pour AlgorithmCompiler v1.7.0 — Standard Algorithmique du Baccalauréat Tunisien.*
