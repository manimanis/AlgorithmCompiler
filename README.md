# AlgorithmCompiler 🚀

**AlgorithmCompiler** est un interpréteur, vérificateur sémantique statique et transpilateur haute performance pour le pseudo-code algorithmique standard utilisé dans l'enseignement secondaire et supérieur en Tunisie (programmes du Baccalauréat tunisien : Mathématiques, Sciences Expérimentales, Sciences de l'Informatique, Technique et Économie-Gestion).

Le projet est développé en **C++20 moderne** et dispose d'une suite complète de **185 tests unitaires automatisés validés à 100%**.

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
│   ├── ast.hpp                 # Arbre Syntaxique Abstrait (AST en C++20)
│   ├── common.hpp              # Utilitaires de chaîne, encodage UTF-8, erreurs
│   ├── environment.hpp         # Environnement d'exécution et table des symboles
│   ├── interpreter.hpp         # Moteur d'interprétation et exécution dynamique
│   ├── lexer.hpp               # Analyseur lexical (tokens, mots-clés, symboles)
│   ├── parser.hpp              # Analyseur syntaxique descendant récursif
│   ├── python_transpiler.hpp   # Transpilateur vers code Python idiomatique
│   ├── semantic_analyzer.hpp   # Analyseur sémantique statique & diagnostics
│   ├── token.hpp               # Définitions des types de jetons (Token)
│   └── value.hpp               # Modèle de valeur polymorphe (entier, réel, chaîne, tableau, enregistrement, fichier)
├── src/
│   ├── main.cpp                # Point d'entrée de l'exécutable algo.exe
│   ├── semantic_analyzer.cpp   # Implémentation de la vérification sémantique
│   ├── python_transpiler.cpp   # Implémentation du transpilateur Python
│   ├── interpreter.cpp         # Implémentation de l'interpréteur
│   ├── parser.cpp              # Implémentation du parseur
│   ├── lexer.cpp               # Implémentation du lexer
│   └── test_runner.cpp         # Runner autonome des 185 tests unitaires
├── tests/                      # 185 tests couvrant toutes les fonctionnalités
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

## 🛠 Installation & Compilation

### Prérequis
- Système d'exploitation : Windows (PowerShell / CMD), Linux ou macOS.
- Compilateur C++ supportant C++20 (`g++` via MSYS2 UCRT64 / MinGW-w64, ou `clang++`).
- Node.js (facultatif, pour les scripts d'automatisation des tests).

### Compilation sous Windows avec MSYS2 GCC
```cmd
build.bat
```
ou via le script Node.js :
```cmd
node scripts/compile.js
```

Les exécutables générés sont :
- `algo.exe` : Le compilateur / interpréteur / transpilateur principal.
- `run_tests.exe` : Le runner de la suite de tests unitaires.

---

## 🚀 Utilisation en Ligne de Commande (CLI)

### Exécuter un algorithme (Interpréteur)
```bash
./algo.exe mon_programme.alg
```

### Transpiler vers Python
Afficher le code Python généré dans la console :
```bash
./algo.exe mon_programme.alg --to-python
```
Enregistrer le code Python directement dans un fichier :
```bash
./algo.exe mon_programme.alg --to-python sortie.py
```

### Options d'exécution
- `--max-iter <N>` : Fixer la limite d'itérations maximale (prévention des boucles infinies).
- `--seed <N>` : Définir la graine pour le générateur pseudo-aléatoire.

---

## 🧪 Suite de Tests Automatisée

AlgorithmCompiler dispose d'une suite de **185 tests unitaires** couvrant l'intégralité des fonctionnalités :

```bash
./run_tests.exe
```

Résultats :
```text
========================================================
                  RÉSULTATS DES TESTS                   
========================================================
 Total tests : 185
 Réussis     : 185
 Échoués     : 0 (100% SUCCÈS)
========================================================
```

Pour exécuter les tests du transpilateur Python :
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
