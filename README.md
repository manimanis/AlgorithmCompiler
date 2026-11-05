# AlgorithmCompiler 🚀

**AlgorithmCompiler** est un interpréteur, vérificateur sémantique statique et transpilateur haute performance pour le pseudo-code algorithmique standard utilisé dans l'enseignement secondaire et supérieur en Tunisie (programmes du Baccalauréat tunisien : Mathématiques, Sciences Expérimentales, Sciences de l'Informatique, Technique et Économie-Gestion).

Le projet est développé en **C++20 moderne** et dispose d'une suite complète de **187 tests unitaires automatisés validés à 100%**.

---

## 📑 Sommaire

1. [Vue d'ensemble & Architecture](#-vue-densemble--architecture)
2. [Fonctionnalités du Langage](#-fonctionnalités-du-langage)
   - [Structure générale d'un algorithme](#structure-générale-dun-algorithme)
   - [Tables de Déclarations (TDO, TDOL, TDNT, TDOG)](#tables-de-déclarations-tdo-tdol-tdnt-tdog)
   - [Types de Données Primitifs & Littéraux](#types-de-données-primitifs--littéraux)
   - [Notation Scientifique](#notation-scientifique)
   - [Structures de Données Avancées (Tableaux, Matrices, Enregistrements)](#structures-de-données-avancées)
   - [Fichiers Textuels & Typés](#fichiers-textuels--typés)
   - [Structures de Contrôle](#structures-de-contrôle)
   - [Sous-programmes (Procédures & Fonctions)](#sous-programmes-procédures--fonctions)
   - [Bibliothèque Standard & Fonctions Prédéfinies](#bibliothèque-standard--fonctions-prédéfinies)
3. [Analyse Sémantique Statique & Diagnostics Pédagogiques](#-analyse-sémantique-statique--diagnostics-pédagogiques)
4. [Transpilateur Python Optimisé](#-transpilateur-python-optimisé)
5. [Installation & Compilation](#-installation--compilation)
6. [Utilisation en Ligne de Commande (CLI)](#-utilisation-en-ligne-de-commande-cli)
7. [Suite de Tests Automatisée](#-suite-de-tests-automatisée)
8. [Exemples d'Algorithmes](#-exemples-dalgorithmes)

---

## 🏛 Vue d'ensemble & Architecture

Le projet est structuré selon les standards des compilateurs modernes :

```text
AlgorithmCompiler/
├── include/
│   ├── arena.hpp               # Allocateur d'arène (Bump/Arena Allocator pour l'AST)
│   ├── ast.hpp                 # Arbre Syntaxique Abstrait & Interfaces Visiteur (ExprVisitor, StmtVisitor)
│   ├── bytecode.hpp            # Définitions des Opcodes, Chunks et BytecodeProgram
│   ├── bytecode_compiler.hpp   # Compilateur AST vers Bytecode linéaire
│   ├── common.hpp              # Utilitaires de chaîne, encodage UTF-8, erreurs
│   ├── environment.hpp         # Environnement d'exécution et table des symboles
│   ├── interpreter.hpp         # Moteur d'interprétation AST via le patron Visiteur
│   ├── lexer.hpp               # Analyseur lexical (tokens, mots-clés, symboles)
│   ├── parser.hpp              # Analyseur syntaxique descendant récursif
│   ├── python_transpiler.hpp   # Transpilateur vers code Python idiomatique
│   ├── semantic_analyzer.hpp   # Analyseur sémantique statique & diagnostics
│   ├── token.hpp               # Définitions des types de jetons (Token)
│   ├── value.hpp               # Modèle de valeur polymorphe (entier, réel, chaîne, tableau, enregistrement, fichier)
│   └── vm.hpp                  # Machine Virtuelle à pile haute performance (Bytecode VM)
├── src/
│   ├── main.cpp                # Point d'entrée CLI (algo / algo.exe) avec options --vm, --ast, --dump-bytecode
│   ├── bytecode.cpp            # Gestion des chunks de bytecode et désassembleur
│   ├── bytecode_compiler.cpp   # Implémentation de la compilation AST -> Bytecode
│   ├── vm.cpp                  # Implémentation du moteur de la machine virtuelle
│   ├── semantic_analyzer.cpp   # Implémentation de la vérification sémantique
│   ├── python_transpiler.cpp   # Implémentation du transpilateur Python
│   ├── interpreter.cpp         # Implémentation de l'interpréteur par Visiteur
│   ├── parser.cpp              # Implémentation du parseur avec allocation d'arène
│   ├── lexer.cpp               # Implémentation du lexer
│   └── test_runner.cpp         # Runner autonome des 187 tests unitaires
├── Makefile                    # Makefile universel multiplateforme (Linux, macOS, Windows)
├── build.sh                    # Script shell de compilation directe pour Linux / macOS
├── build.bat                   # Script batch de compilation pour Windows
├── tests/                      # 187 tests couvrant toutes les fonctionnalités
└── scripts/                    # Scripts Node.js pour compilation et validation
```

---

## 💻 Fonctionnalités du Langage

### Structure générale d'un algorithme

Un programme algorithmique se compose d'une entête, de déclarations de sous-programmes éventuelles, d'un bloc principal et des tables de déclarations :

```algorithme
Algorithme Nom_Programme
Début
    // Instructions principales
Fin

TDO
Objet : Type/Nature
// Déclarations des objets globaux
```

### Tables de Déclarations (TDO, TDOL, TDNT, TDOG)

- **`TDO`** : Tableau de Déclaration des Objets globaux.
- **`TDOL`** : Tableau de Déclaration des Objets Locaux (associé à chaque procédure ou fonction).
- **`TDNT`** : Tableau de Déclaration des Nouveaux Types personnalisés (enregistrements, types tableaux, types matrices).
- **`TDOG`** : Déclaration des procédures et fonctions globales.

### Types de Données Primitifs & Littéraux

| Type algorithmique | Équivalent C++ / Python | Exemple de valeur |
| :--- | :--- | :--- |
| `entier` | `int64_t` / `int` | `42`, `-10`, `0` |
| `réel` / `reel` | `double` / `float` | `3.14`, `-0.001`, `1e-9` |
| `chaîne` / `chaine` | `std::string` / `str` | `"Bonjour"`, `""` |
| `caractère` / `caractere` | `std::string` (1 car.) | `'A'`, `'9'`, `'\n'` |
| `booléen` / `booleen` | `bool` / `bool` | `Vrai`, `Faux` (ou `True`, `False`) |

### Notation Scientifique

AlgorithmCompiler supporte nativement la notation exponentielle/scientifique dans les littéraux et les calculs :
- Littéraux : `1e-9`, `2.5e3`, `1.2E+6`, `1e-05`.
- Opérations directes : `tau ← R * C * 1e-9`.

### Structures de Données Avancées

#### 1. Tableaux 1D
Déclaration dans le `TDO` ou via un type `TDNT` :
```algorithme
TDNT
tab = tableau de 10 entier

TDO
Objet : Type/Nature
t : tab
notes : Tableau de 20 réel
```
- Accès et affectation par crochet : `t[0] ← 15`, `x ← notes[i]`.
- Vérification statique du type d'indice (entier obligatoire).
- Détection des accès hors bornes à l'exécution.

#### 2. Matrices 2D
Prise en charge de la syntaxe standard tunisienne :
```algorithme
TDNT
mat = Tableau de 3 lignes * 4 colonnes de réel
// ou : Tableau de 3, 4 de entier

TDO
Objet : Type/Nature
M : mat
```
- Accès par double crochet ou virgule : `M[i][j]` ou `M[i, j]`.

#### 3. Enregistrements (Records)
Définition via `TDNT` ou directement en `TDO` / `TDOL` :
```algorithme
TDNT
Etudiant = Enregistrement
    nom : chaîne
    age : entier
    moyenne : réel
Fin

TDO
Objet : Type/Nature
e1, e2 : Etudiant
```
- Accès aux champs par point : `e1.nom ← "Sami"`, `e1.age ← 18`.
- Copie globale d'enregistrement : `e2 ← e1`.

#### 4. Enregistrements Imbriqués & Combinaisons
- **Enregistrements imbriqués** : Un enregistrement contenant un autre enregistrement (`personne.adresse.ville`).
- **Tableau d'enregistrements** : `promo[i].nom ← "Mouna"`.
- **Enregistrement contenant des tableaux** : `e.notes[0] ← 17.5`.
- **Passage par référence d'enregistrements imbriqués** vers des procédures : `@p.adresse`.

### Fichiers Textuels & Typés

#### Fichiers Textes (`Fichier Texte`)
```algorithme
Ouvrir(f, "notes.txt", "w") // Modes supportés : "r" (lecture), "w" (écriture), "a" (ajout)
Ecrire(f, "Bonjour le monde")
Fermer(f)

Ouvrir(f, "notes.txt", "r")
Tant Que Non Fin_Fichier(f) Faire
    Lire_Ligne(f, ligne)
    Ecrire(ligne)
Fin Tant Que
Fermer(f)
```

#### Fichiers Typés de Données (`Fichier d'<Type>`)
- Stockage binaire d'entiers, réels et enregistrements complexes.
- Lecture et écriture directes d'enregistrements imbriqués : `Lire(f, etudiant)`, `Ecrire(f, etudiant)`.

### Structures de Contrôle

1. **Conditionnelle `Si`** :
   ```algorithme
   Si moyenne >= 10 Alors
       Ecrire("Admis")
   Sinon Si moyenne >= 9 Alors
       Ecrire("Rachat")
   Sinon
       Ecrire("Ajourné")
   Fin Si
   ```

2. **Aiguillage Multiple `Selon`** :
   Supporte les valeurs scalaires, listes et intervalles :
   ```algorithme
   Selon code Faire
       Cas 1, 2, 3 : Ecrire("Priorité haute")
       Cas 4 .. 7  : Ecrire("Priorité moyenne")
       Autrement   : Ecrire("Inconnu")
   Fin Selon
   ```

3. **Boucle `Pour`** :
   ```algorithme
   Pour i de 0 à 9 Faire
       Ecrire(i)
   Fin Pour

   Pour k de 10 à 0 Pas -2 Faire
       Ecrire(k)
   Fin Pour
   ```

4. **Boucle `Tant Que`** :
   ```algorithme
   Tant Que x > 0 Faire
       x ← x - 1
   Fin Tant Que
   ```

5. **Boucle `Répéter ... Jusqu'à`** :
   Supporte les conditions simples et les comparaisons chaînées :
   ```algorithme
   Répéter
       Ecrire("Donner E : ") ; Lire(E)
   Jusqu'à 0 < E ≤ 50
   ```

### Sous-programmes (Procédures & Fonctions)

- **Procédures** : Définies avec `Procédure Nom(paramètres) ... Fin`.
- **Fonctions** : Définies avec `Fonction Nom(paramètres) : TypeRetour ... Fin`.
- **Passage de paramètres** :
  - Par valeur : `x : entier`
  - Par référence : `@x : réel` (modifie directement la variable de l'appelant).
- **Récursivité** supportée sans limitation artificielle.

### Bibliothèque Standard & Fonctions Prédéfinies

| Fonction | Description |
| :--- | :--- |
| `Racine(x)` | Racine carrée ($\sqrt{x}$) |
| `Carre(x)` | Carré ($x^2$) |
| `Abs(x)` | Valeur absolue ($\|x\|$) |
| `Exp(x)` | Exponentielle ($e^x$) |
| `Ln(x)` / `Log(x)` | Logarithme népérien / décimal |
| `Ent(x)` | Partie entière |
| `Arrondi(x)` | Arrondi à l'entier le plus proche |
| `Long(ch)` | Longueur d'une chaîne |
| `Chr(code)` | Caractère correspondant au code ASCII |
| `Ord(c)` | Code ASCII d'un caractère |
| `ConvCh(val)` | Conversion d'un nombre en chaîne |
| `Valeur(ch)` | Conversion d'une chaîne en nombre |
| `Sous_Chaine(ch, deb, fin)` | Extraction de sous-chaîne |
| `Pos(sous_ch, ch)` | Position de la première occurrence |
| `Majus(ch)` / `Minus(ch)` | Conversion en majuscules / minuscules |
| `Effacer(ch, deb, n)` | Suppression de caractères dans une chaîne |
| `Alea(min, max)` | Nombre pseudo-aléatoire dans $[min, max]$ |
| `Graine(seed)` | Initialisation de la graine aléatoire pour reproductibilité |
| `Fin_Fichier(f)` | Test de fin de fichier |
| `EstNum(ch)` / `Est_Num(ch)` | Vérification si une chaîne est numérique |

---

## 🎯 Analyse Sémantique Statique & Diagnostics Pédagogiques

AlgorithmCompiler intègre un analyseur sémantique statique conçu spécifiquement pour l'apprentissage. Il détecte les erreurs avant l'exécution avec un niveau de clarté similaire à **Rust** ou **Clang** :

- **Indication explicite de l'objet non défini** : Chaque message nomme expressément la variable, la procédure, la fonction ou le champ fautif.
- **Prévisualisation du code source** avec numéro de ligne, colonne et pointeur visuel (`^~~~`).
- **Suggestions intelligentes (Distance de Levenshtein)** : Propose la correction la plus probable en cas de faute de frappe (*"Vouliez-vous dire '...' ?"*).
- **Vérifications sémantiques effectuées** :
  - Variables, fonctions et procédures non déclarées dans le `TDO`/`TDOL`/`TDNT`.
  - Incompatibilité des types lors de l'affectation.
  - Conditions non booléennes (`Si`, `Tant Que`, `Jusqu'à`).
  - Passages invalides aux paramètres par référence `@param` (nécessite une LValue modifiable).
  - Validation de l'arité (nombre exact d'arguments attendus vs fournis).
  - Types d'indices de tableaux invalides (non entiers).
  - Accès à des champs inexistants dans les enregistrements.
  - Fonctions sans instruction de retour `Retourner`.
  - Bornes ou pas non numériques dans les boucles `Pour`.

#### Exemple de diagnostic visuel :
```text
Erreur sémantique (ligne 4, colonne 10) :
      Ecrire(totla + 1)
             ^~~~~
La variable 'totla' non définie (objet non déclaré dans le TDO/TDOL).
Suggestion : Vouliez-vous dire 'total' ? (ligne 4)
```

---

## 🐍 Transpilateur Python Optimisé

AlgorithmCompiler permet de générer du code **Python 3 moderne, propre et directement exécutable** via l'option `--to-python` :

- **Aucune dépendance superflue** : N'importe que les fonctions strictement nécessaires (ex. `from math import exp`, `from numpy import array`).
- **Absence de fonctions d'assistance inutiles** : Aucun helper artificiel tel que `_fmt`, `_valeur`, etc.
- **Règle stricte sur `break`** : L'instruction `break` étant bannie pédagogiquement, la structure `Répéter ... Jusqu'à` est transpilée de manière idiomatique sans `while True: ... break`.
- **Tableaux et Matrices via NumPy** : Implémentés exclusivement via la fonction `array` de NumPy (`from numpy import array`).
- **Enregistrements via `dict`** : Les enregistrements sont transpilés en dictionnaires natifs Python (`dict`).
- **Fonctions natives directes** : `Pos(ch1, ch2)` est directement transpilé en `ch2.find(ch1)`.

---

## ⚡ Architecture Haute Performance

AlgorithmCompiler intègre une architecture de pointe inspirée des compilateurs modernes :

### 1. Allocateur d'Arène (Arena / Bump Allocator)
- **Allocation continue par blocs** : Tous les nœuds de l'AST (`Expr`, `Stmt`) sont alloués au sein d'une arène de mémoire contiguë (`Arena`), supprimant les milliers d'allocations de petits blocs dispersés sur le tas.
- **Libération instantanée O(1)** : À la destruction de l'objet `Program`, l'intégralité de la mémoire des nœuds est désallouée en bloc, éliminant la fragmentation mémoire et accélérant les temps d'analyse.

### 2. Patron Visiteur (Visitor Pattern)
- **Suppression du surcoût RTTI** : Remplacement des cascades de `std::dynamic_pointer_cast` par le patron Visiteur (`ExprVisitor`, `StmtVisitor`).
- **Dispatch virtuel O(1)** : L'interpréteur et le compilateur visitent directement les nœuds via `accept(*this)` en temps constant, assurant une traversée fluide et sans coût de vérification de type dynamique.

### 3. Machine Virtuelle à Bytecode (Bytecode VM)
- **Exécution linéaire ultra-rapide** : Le compilateur de bytecode (`BytecodeCompiler`) transforme l'AST en un flux d'instructions compactes (`Chunk`).
- **Machine à pile optimisée** : La VM (`algo::VM`) exécute le bytecode directement via une boucle d'interprétation optimisée, divisant drastiquement les temps d'exécution des algorithmes itératifs lourds.
- **Désassemblage transparent (`--dump-bytecode`)** : Visualisez pas à pas le code machine virtuel généré (opcodes, pile, constantes, offsets de saut).

---

## 🛠 Installation & Compilation

### Prérequis
- **Système d'exploitation** : Linux (Ubuntu, Debian, Fedora, Arch...), macOS, ou Windows (MSYS2 / MinGW-w64).
- **Compilateur C++** : Supportant **C++20** (`g++` >= 10 ou `clang++` >= 13).
- **Make** : GNU Make (recommandé pour la compilation incrémentale).
- **Node.js** (facultatif) : Pour exécuter les scripts de validation automatisée.

---

### Compilation sous Linux / macOS

#### Méthode 1 : Avec `make` (Recommandé)
Le `Makefile` est entièrement configuré pour détecter votre environnement et compiler en parallèle :
```bash
# Compilation des deux exécutables (algo et run_tests)
make -j$(nproc)

# Nettoyer les objets et binaires compilés
make clean

# Lancer la suite complète des tests unitaires
make test
```

#### Méthode 2 : Avec le script Bash `build.sh`
```bash
chmod +x build.sh
./build.sh
```

#### Méthode 3 : Via le script Node.js
```bash
node scripts/compile.js
```

Les exécutables générés sous Linux sont :
- `algo` : Compilateur, interpréteur et transpilateur principal.
- `run_tests` : Runner autonome des tests unitaires.

---

### Cross-Compilation avec le Makefile 🌍

Le [`Makefile`](file:///c:/xampp-school/htdocs/MesProjets/AlgorithmCompiler/Makefile) intègre le support natif complet de la **cross-compilation multi-plateforme** :

#### 1. Cross-compiler vers Windows depuis Linux / macOS (ou CI/CD)
Pour générer des exécutables Windows (`algo.exe` et `run_tests.exe`) 100% autonomes (liés statiquement avec `-static -static-libgcc -static-libstdc++`) sans dépendance DLL :
```bash
# Avec détection automatique du compilateur MinGW (x86_64-w64-mingw32-g++)
make windows

# Ou en spécifiant explicitement le préfixe de la toolchain
make CROSS_COMPILE=x86_64-w64-mingw32-
```

Pour cibler Windows 32-bit :
```bash
make win32
# Équivalent à : make TARGET_OS=windows CROSS_COMPILE=i686-w64-mingw32-
```

#### 2. Compiler pour Linux (binaire natif ELF)
```bash
make linux
```

#### 3. Afficher l'aide interactive du Makefile
```bash
make help
```

---

### Compilation sous Windows

#### Méthode 1 : Script Batch natif (Recommandé)
Le script [`build.bat`](build.bat) configure automatiquement le `PATH` vers votre environnement MSYS2 (`C:\msys64\ucrt64\bin` ou `C:\MSYS2\ucrt64\bin`), compile avec liaison statique (`-static`) et génère les exécutables :
```cmd
build.bat
```

#### Méthode 2 : Avec GNU Make (`mingw32-make` ou `make`)
Si vous utilisez l'invite de commandes Windows (`cmd.exe`), configurez d'abord le chemin vers les outils MSYS2 dans votre session :
```cmd
set PATH=C:\MSYS2\ucrt64\bin;%PATH%
mingw32-make
```
*(ou `mingw32-make test` pour compiler et exécuter les tests).*

*Astuce : Si vous utilisez directement le terminal **MSYS2 UCRT64** (ou Git Bash), le `PATH` est déjà configuré par défaut, il suffit de taper :*
```bash
make
```

#### Méthode 3 : Via le script Node.js
```cmd
node scripts/compile.js
```

Les exécutables générés sous Windows sont :
- `algo.exe` : Le compilateur, interpréteur et transpilateur principal (autonome, sans dépendance DLL).
- `run_tests.exe` : Le runner de la suite des 187 tests unitaires.

---

## 🚀 Utilisation en Ligne de Commande (CLI)

### Exécuter un algorithme (Interpréteur)
Sous Linux / macOS :
```bash
./algo mon_programme.alg
```
Sous Windows :
```cmd
algo.exe mon_programme.alg
```

### Transpiler vers Python
Afficher le code Python généré dans le terminal :
```bash
./algo mon_programme.alg --to-python
```
Enregistrer le code Python directement dans un fichier :
```bash
./algo mon_programme.alg --to-python sortie.py
```

### Options de la ligne de commande
- `-h, --help` : Affiche l'aide détaillée et les exemples d'utilisation.
- `-v, --version` : Affiche le numéro de version du compilateur.
- `--check` : Effectue uniquement l'analyse syntaxique et la vérification sémantique statique sans exécuter le code.
- `--vm` : Exécute l'algorithme via la Machine Virtuelle à Bytecode (mode activé par défaut, exécution linéaire ultra-rapide).
- `--ast` : Force l'exécution via l'Interpréteur d'AST classique (Patron Visiteur sans RTTI).
- `--dump-bytecode` : Compile et affiche le désassemblage complet du bytecode généré sans exécuter.
- `--to-python, --py [out.py]` : Transpile vers du code Python 3 (vers la sortie standard ou un fichier).
- `--max-iter <N>` : Fixe la limite maximale d'itérations de boucle (défaut : 1 000 000).
- `--timeout <ms>` : Fixe le délai maximal d'exécution en millisecondes (défaut : 50 000 ms).
- `--seed <N>` : Définit la graine pseudo-aléatoire pour les fonctions Aléa / Hasard.

---

## 🧪 Suite de Tests Automatisée

AlgorithmCompiler dispose d'une suite de **187 tests unitaires** couvrant l'intégralité des fonctionnalités du langage.

### Lancer tous les tests en C++
Sous Linux / macOS :
```bash
make test
# ou directement :
./run_tests
```

Sous Windows :
```cmd
run_tests.exe
```

Résultats attendus :
```text
========================================================
                  RÉSULTATS DES TESTS                   
========================================================
 Total tests : 187
 Réussis     : 187
 Échoués     : 0 (100% SUCCÈS)
========================================================
```

### Lancer les tests du transpilateur Python (facultatif)
Nécessite Node.js et Python 3 :
```bash
node scripts/test_transpiler.js
```
```text
=== TEST DU TRANSPILATEUR PYTHON ===
Results: 30/30 passed
```

---

## 📖 Exemples d'Algorithmes

### 1. Étude d'un Circuit RC (Notation scientifique & Fonctions)

```algorithme
Procédure Saisir_Donnees(@E: réel, @C: réel, @R: réel)
Début
  Répéter
    Ecrire("Tension du générateur :") ; Lire(E)
  Jusqu'à 0 < E ≤ 50
  Répéter
    Ecrire("Capacité du condensateur :") ; Lire(C)
  Jusqu'à 10 ≤ C ≤ 2700000
  Répéter
    Ecrire("Résistance du résistor :") ; Lire(R)
  Jusqu'à 10 ≤ R ≤ 1000000
Fin

Fonction Calcul_Tau(R: réel, C: réel):réel
Début
  Retourner R * C * 1e-9
Fin

Algorithme Etude_Circuit_RC
Début
  Saisir_Donnees(E, C, R)
  tau ← Calcul_Tau(R, C)
  Ecrire("Constante de temps tau =", tau, "s")
Fin

TDO
Objet : Type/Nature
E, C, R, tau : réel
Saisir_Donnees : Procédure
Calcul_Tau : Fonction
```

### 2. Gestion d'étudiants (Enregistrements & Tableaux)

```algorithme
Algorithme Gestion_Etudiants
Début
  promo[0].nom ← "Sami"
  promo[0].age ← 18
  promo[1].nom ← "Mouna"
  promo[1].age ← 19
  Pour i De 0 A 1 Faire
    Ecrire(promo[i].nom, "a", promo[i].age, "ans")
  Fin Pour
Fin

TDNT
Etudiant = Enregistrement
  nom : chaîne
  age : entier
Fin

TDO
Objet : Type/Nature
promo : Tableau de 2 Etudiant
i : entier
```

---

## 📄 Licence

Ce projet est sous licence MIT - libre pour usage pédagogique, académique et professionnel.
